#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "knxprod.h"

#define KDEBUG_Received

#define proc_crc8(u8CRC, u8Data) (u8CRC8Table[u8CRC ^ u8Data])

uint8_t EnoceanModule::u8CRC8Table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e,
    0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb,
    0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd, 0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8,
    0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6,
    0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d,
    0x9a, 0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50,
    0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80, 0x95,
    0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4, 0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f,
    0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a,
    0x33, 0x34, 0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6A, 0x6d, 0x64, 0x63, 0x3e,
    0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83, 0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc,
    0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3};

// Konstruktor: keine eigene Initialisierung nötig, Rest übernimmt die Basisklasse.
EnoceanModule::EnoceanModule() {}

// Liefert den Modulnamen "Enocean" für Log-/Konsolenausgaben.
const std::string EnoceanModule::name() { return "Enocean"; }

// Liefert bewusst einen leeren String, damit das Modul nicht separat in der Versionsausgabe der Konsole erscheint.
const std::string EnoceanModule::version() {
  // hides the module in the version output on the console, because the firmware
  // version is sufficient.
  return "";
}

// Merkt sich den zu verwendenden UART-Stream für die Kommunikation mit dem EnOcean-Transceiver.
void EnoceanModule::initSerial(Stream &serial) { _serial = &serial; }

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
  if (delayCheck(_timer1, 5000)) {
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
  if (millis() - lastTime > 5000) {
    logInfoP("Alive, runtime: %u", lastTime);
    lastTime = millis();
  }

  // EnOcean IN -> KNX OUT
  u8RetVal = ENOCEAN_NO_RX_TEL;
  u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
  getEnOceanMSG(u8RetVal, &m_Pkt_st);
}

// Einmalige Initialisierung: liest die Transceiver-Base-ID aus (Basis-ID-Vergleich und Repeater-Funktion sind auskommentiert).
void EnoceanModule::begin() {
  if (isInited)
    return;

  m_Pkt_st.u8DataBuffer = &u8datBuf[0];

  //****************** Read, Check & Set EnOcean Gateway Base ID
  //************************************
  // communicates via Enocean UART channel
  // 1.) read base-ID
  delay(2000);
  logInfoP("read BaseID");
  readBaseId(&lui8_BaseID_p[0]);

  
  // 2.) compare old base-ID with the new ID
  if ((knx.paramByte(ENO_SetBaseIdFunc) >> ENO_SetBaseIdFuncShift) & 1)
  {
    if (checkBaseID())
    { // old != new
      logInfoP("Base-ID: OLD != NEW -> change! ");
      setBaseId(&lui8_BaseID_p[0]);
      // 3.) read Base-ID again and print it out
      readBaseId(&lui8_BaseID_p[0]);
    }
    else // old == new
    {
      logDebugP("Base-ID: OLD == NEW -> NO change! ");
    }
  }
 

  for (int i = 0; i < BASEID_BYTES; i++) {
    logInfoP("Base-ID: %i", lui8_BaseID_p[i], HEX);
  }

  //****************** Repeater Function ************************************
  
  #ifdef KDEBUG
    logDebugP("----------------------");
  #endif
    setRepeaterFunc();
    // prüft ob Änderungen umgesetzt wurden
    readRepeaterFunc();

  #ifdef KDEBUG
    logDebugP("----------------------");
  #endif
  

  isInited = true;
}

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

void EnoceanModule::setBaseId(uint8_t *fui8_BaseID_p)
{
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf[5];
  // uint8_t loopCount = 0;

  lu8SndBuf[0] = u8CO_WR_IDBASE;
  lu8SndBuf[1] = 0xFF;
  lu8SndBuf[2] = knx.paramByte(ENO_Id2);
  lu8SndBuf[3] = knx.paramByte(ENO_Id4);
  lu8SndBuf[4] = knx.paramByte(ENO_Id6);

  lRdBaseIDPkt_st.u16DataLength = 0x0005;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (set base ID).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st))
  {
    u8RetVal = ENOCEAN_NO_RX_TEL;
    logInfoP("Receiving telegram (set base ID).");
    while (u8RetVal == ENOCEAN_NO_RX_TEL)
    {
      u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
    }

    switch (u8RetVal)
    {
    case ENOCEAN_OK:
    {
      {
        std::string hexStr;
        char hexByte[4];
        for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++)
        {
          // SERIAL_PORT.print("%X", m_Pkt_st.u8DataBuffer[i]);
          sprintf(hexByte, "%02X ", m_Pkt_st.u8DataBuffer[i]);
          hexStr += hexByte;
        }
        logInfoP("Data: %s", hexStr.c_str());
      }

      switch (m_Pkt_st.u8Type)
      {
      case u8RESPONSE:
      {
        switch (m_Pkt_st.u8DataBuffer[0])
        {
        case 0x00:
          logInfoP("Received Response: RET_OK");
          break;
        case 0x02:
          logInfoP("Received Response: RET_NOT_SUPPORTED");
          break;
        case 0x82:
          logInfoP("Received Response: FLASH_HW_ERROR");
          break;
        case 0x90:
          logInfoP("Received Response: BASEID_OUT_OF_RANGE");
          break;
        case 0x91:
          logInfoP("Received Response: BASEID_MAX_REACHED");
          break;

        default:
          break;
        }
      }
      break;
      default:
      {
        /// SERIAL_PORT.println("%X", m_Pkt_st.u8Type);
        logInfoP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      }
    }
    break;
    case ENOCEAN_NO_RX_TEL:
      logInfoP("ERROR Receiving telegram (set base ID).");
      break;
    default:
    {
      logInfoP("set receiving base ID");
    }
    } // ENDE SWITCH
  }
}

// Fragt beim Transceiver die Basis-ID per ESP3-Common-Command an und wartet blockierend auf die Antwort.
void EnoceanModule ::readBaseId(uint8_t *fui8_BaseID_p) {
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf = u8CO_RD_IDBASE;
  // uint8_t loopCount = 0;

  lRdBaseIDPkt_st.u16DataLength = 0x0001;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf;

  // Swap data length bytes to little endian
  // uint8_t temp;
  // temp = lRdBaseIDPkt_st.u16DataLength[0];
  // lRdBaseIDPkt_st.u16DataLength[0] = lRdBaseIDPkt_st.u16DataLength[1];
  // lRdBaseIDPkt_st.u16DataLength[1] = temp;

  logDebugP("Sending telegram (read base ID).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st)) {
    u8RetVal = ENOCEAN_NO_RX_TEL;
    logDebugP("Receiving telegram (read base ID).");
    while (u8RetVal == ENOCEAN_NO_RX_TEL) {
      u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
    }

    switch (u8RetVal) {
    case ENOCEAN_OK:
      for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++) {
        logDebugP("Data: %u", m_Pkt_st.u8DataBuffer[i], HEX);
      }
      switch (m_Pkt_st.u8Type) {
      case u8RESPONSE:
        logDebugP("Received Response.");
        for (int i = 0; i < BASEID_BYTES; i++) {
          memcpy((void *)&(fui8_BaseID_p[i]), (void *)&(m_Pkt_st.u8DataBuffer[i + 1]), 1);
        }
        break;
      default:
        logDebugP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      break;
    case ENOCEAN_NO_RX_TEL:
      logDebugP("ERROR Receiving telegram (read base ID).");
      break;
    default:
      logDebugP("Error receiving base ID");
    }
  }
}

// Rückweg KNX→EnOcean: ermittelt Kanal und lokalen KO-Index des beschriebenen Gruppenobjekts über die ENO_KoCalc*-Makros.
void EnoceanModule::processInputKo(GroupObject &iKo) {
  int channel = ENO_KoCalcChannel(iKo.asap());
  if (channel < 0 || channel >= ParamENO_VisibleChannels)
    return; // Gruppenobjekt gehört zu keinem aktiven EnOcean-Kanal

  uint8_t _channelIndex = (uint8_t)channel;
  int koIndex = ENO_KoCalcIndex(iKo.asap());

  logDebugP("processInputKo: Kanal %u, KO-Index %d", _channelIndex, koIndex);
  handleKnxEvent(_channelIndex, koIndex, iKo);
}

// Wird aufgerufen, wenn ein Gruppenobjekt eines EnOcean-Kanals von der KNX-Seite beschrieben wurde; _channelIndex ist der betroffene Kanal, koIndex die lokale
// Position (0-basiert) innerhalb des Kanal-Blocks, ko der Wert.
void EnoceanModule::handleKnxEvent(uint8_t _channelIndex, int koIndex, GroupObject &iKo) {
  logDebugP("handleKnxEvent: KO-Index %d, Value:", koIndex);
  logHexDebugP(iKo.valueRef(), iKo.valueSize());
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

// ESP3-Empfangs-Statemachine: liest byteweise vom UART, prüft Sync/CRC8 und liefert ein vollständiges Paket oder einen Fehlerstatus.
uint8_t EnoceanModule::uart_getPacket(PACKET_SERIAL_TYPE_ *pPacket, uint16_t u16BufferLength) {

  //! UART received byte code
  uint8_t u8RxByte;
  //! Checksum calculation
  static uint8_t u8CRC = 0;
  //! Nr. of bytes received
  static uint16_t u16Count = 0;
  //! State machine counter
  // static STATES_GET_PACKET_ u8State = GET_SYNC_STATE;
  //! Timeout measurement
  // static uint8_t u8TickCount = 0;
  // Byte buffer pointing at the paquet address
  uint8_t *u8Raw = (uint8_t *)pPacket;
  // Temporal variable
  uint8_t i;
  // Check for timeout between two bytes
  // TODO
  // if (((uint8)ug32SystemTimer) - u8TickCount > SER_INTERBYTE_TIME_OUT)
  //{
  // Reset state machine to init state
  // u8State = GET_SYNC_STATE;
  //}
  // State machine goes on when a new byte is received
  if (_serial->available() > 0) {
    while (_serial->readBytes(&u8RxByte, 1) == 1) {
      // Comment out for debugging
      // Serial.println(u8RxByte, HEX);

      // Tick count of last received byte
      // TODO
      // u8TickCount = (uint8)ug32SystemTimer;
      // State machine to load incoming packet bytes
      switch (u8State) {
      // Waiting for packet sync byte 0x55ENOModule::uart_getPacket
      case GET_SYNC_STATE:
        if (u8RxByte == SER_SYNCH_CODE) {
          u8State = GET_HEADER_STATE;
          u16Count = 0;
          u8CRC = 0;
        }
        break;
      // Read the header bytes
      case GET_HEADER_STATE:
        // Copy received data to buffer
        u8Raw[u16Count++] = u8RxByte;
        u8CRC = proc_crc8(u8CRC, u8RxByte);
        // All header bytes received?
        if (u16Count == SER_HEADER_NR_BYTES) {
          // Serial.print("Received all header bytes.\n");
          // Serial.print("pPacket->u16DataLength: ");
          // Serial.println(pPacket->u16DataLength, HEX);
          // Serial.print("u8Raw[1]");
          // Serial.println(u8Raw[1], HEX);
          // Serial.print("u8Raw[2]");
          // Serial.println(u8Raw[2], HEX);
          uint8_t temp;
          temp = u8Raw[0];
          u8Raw[0] = u8Raw[1];
          u8Raw[1] = temp;
          // Serial.print("pPacket->u16DataLength: ");
          // Serial.println(pPacket->u16DataLength, HEX);
          // Serial.print("u8Raw[1]");
          // Serial.println(u8Raw[1], HEX);
          // Serial.print("u8Raw[2]");
          // Serial.println(u8Raw[2], HEX);
          u8State = CHECK_CRC8H_STATE;
        }
        break;
      // Check header checksum & try to resynchonise if error happened
      case CHECK_CRC8H_STATE:
        // Header CRC correct?
        if (u8CRC != u8RxByte) {
          logDebugP("CRC check failed.");
          // No. Check if there is a sync byte (0x55) in the header
          int a = -1;
          for (i = 0; i < SER_HEADER_NR_BYTES; i++)
            if (u8Raw[i] == SER_SYNCH_CODE) {
              // indicates the next position to the sync byte found
              a = i + 1;
              break;
            };
          if ((a == -1) && (u8RxByte != SER_SYNCH_CODE)) {
            // Header and CRC8H does not contain the sync code
            u8State = GET_SYNC_STATE;
            break;
          } else if ((a == -1) && (u8RxByte == SER_SYNCH_CODE)) {
            // Header does not have sync code but CRC8H does.
            // The sync code could be the beginning of a packet
            u8State = GET_HEADER_STATE;
            u16Count = 0;
            u8CRC = 0;
            break;
          }
          // Header has a sync byte. It could be a new telegram.
          // Shift all bytes from the 0x55 code in the buffer.
          // Recalculate CRC8 for those bytes
          u8CRC = 0;
          for (i = 0; i < (SER_HEADER_NR_BYTES - a); i++) {
            u8Raw[i] = u8Raw[a + i];
            u8CRC = proc_crc8(u8CRC, u8Raw[i]);
          }
          u16Count = SER_HEADER_NR_BYTES - a;
          // u16Count = i; // Seems also valid and more intuitive than u16Count
          // -= a; Copy the just received byte to buffer
          u8Raw[u16Count++] = u8RxByte;
          u8CRC = proc_crc8(u8CRC, u8RxByte);
          if (u16Count < SER_HEADER_NR_BYTES) {
            u8State = GET_HEADER_STATE;
            break;
          }
          break;
        }
        // CRC8H correct. Length fields values valid?
        if ((pPacket->u16DataLength + pPacket->u8OptionLength) == 0) {
          // No. Sync byte received?
          if ((u8RxByte == SER_SYNCH_CODE)) {
            // yes
            u8State = GET_HEADER_STATE;
            u16Count = 0;
            u8CRC = 0;
            break;
          }
          // Packet with correct CRC8H but wrong length fields.
          u8State = GET_SYNC_STATE;
          return ENOCEAN_OUT_OF_RANGE;
        }
        // Correct header CRC8. Go to the reception of data.
        u8State = GET_DATA_STATE;
        u16Count = 0;
        u8CRC = 0;
        break;
      // Copy the information bytes
      case GET_DATA_STATE:
        // Copy byte in the packet buffer only if the received bytes have enough
        // room
        if (u16Count < u16BufferLength) {
          pPacket->u8DataBuffer[u16Count] = u8RxByte;
          u8CRC = proc_crc8(u8CRC, u8RxByte);
        }
        // When all expected bytes received, go to calculate data checksum
        if (++u16Count == (pPacket->u16DataLength + pPacket->u8OptionLength)) {
          u8State = CHECK_CRC8D_STATE;
        }

        // Serial.print(u16Count);
        // Serial.println(u16Count, DEC);

        break;
      // Check the data CRC8
      case CHECK_CRC8D_STATE:
        // In all cases the state returns to the first state: waiting for next
        // sync byte
        u8State = GET_SYNC_STATE;
        // Received packet bigger than space to allocate bytes?
        if (u16Count > u16BufferLength)
          return ENOCEAN_OUT_OF_RANGE;
        // Enough space to allocate packet. Equals last byte the calculated
        // CRC8?
        if (u8CRC == u8RxByte)
          return ENOCEAN_OK; // Correct packet received
        // False CRC8.
        // If the received byte equals sync code, then it could be sync byte for
        // next paquet.
        if ((u8RxByte == SER_SYNCH_CODE)) {
          u8State = GET_HEADER_STATE;
          u16Count = 0;
          u8CRC = 0;
        }
        return ENOCEAN_NOT_VALID_CHKSUM;
      default:
        // Yes. Go to the reception of info.
        u8State = GET_SYNC_STATE;
        break;
      }
    }
  }
  return (u8State == GET_SYNC_STATE) ? ENOCEAN_NO_RX_TEL : ENOCEAN_NEW_RX_BYTE;
}

// Sendet ein Paket im ESP3-Format über UART (Sync-Byte, Header inkl. CRC8, Daten inkl. CRC8).
uint8_t EnoceanModule::uart_sendPacket(PACKET_SERIAL_TYPE_ *pPacket) {
  uint16_t i;
  uint8_t u8CRC;
  uint8_t *u8Raw = (uint8_t *)pPacket;
  // When both length fields are 0, then this telegram is not allowed.

  if ((pPacket->u16DataLength || pPacket->u8OptionLength) == 0) {
    return ENOCEAN_OUT_OF_RANGE;
  }

  uint16_t lui16_PacketLength = pPacket->u16DataLength;
  uint8_t temp;
  temp = u8Raw[0];
  u8Raw[0] = u8Raw[1];
  u8Raw[1] = temp;
  // Sync
  while (_serial->write(0x55) != 1)
    ;
  // Header
  _serial->write((char *)pPacket, 4);
  // Header CRC
  u8CRC = 0;
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[0]);
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[1]);
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[2]);
  u8CRC = proc_crc8(u8CRC, ((uint8_t *)pPacket)[3]);
  while (_serial->write(u8CRC) != 1)
    ;
  // Data
  u8CRC = 0;
  for (i = 0; i < lui16_PacketLength + pPacket->u8OptionLength; i++) {
    u8CRC = proc_crc8(u8CRC, pPacket->u8DataBuffer[i]);
    while (_serial->write(pPacket->u8DataBuffer[i]) != 1)
      ;
  }
  // Data CRC
  while (_serial->write(u8CRC) != 1)
    ;
  return ENOCEAN_OK;
}

// Wertet ein empfangenes ERP1-Funktelegramm aus und reicht es an alle konfigurierten Kanäle zur ID-Prüfung/Verarbeitung weiter.
void EnoceanModule::getEnOceanMSG(uint8_t u8RetVal, PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  if (u8RetVal == ENOCEAN_OK) {
#ifdef KDEBUG_Received
    Serial.print(F("Received Data: "));
    for (int i = 0; i < f_Pkt_st->u16DataLength + (uint16_t)f_Pkt_st->u8OptionLength; i++) {
      // Serial.print(F("%X"), f_Pkt_st->u8DataBuffer[i]);
      Serial.print(f_Pkt_st->u8DataBuffer[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println(F(""));
#endif

    if (f_Pkt_st->u8Type == u8RADIO_ERP1) {

#ifdef KDEBUG_min
      if (f_Pkt_st->u8DataBuffer[0] == u8RORG_RPS) {
        logDebugP("RPS: Data:%u ID:%u", f_Pkt_st->u8DataBuffer[1], f_Pkt_st->u8DataBuffer[2], f_Pkt_st->u8DataBuffer[3],
                  f_Pkt_st->u8DataBuffer[4], f_Pkt_st->u8DataBuffer[5], HEX, HEX, HEX, HEX, HEX)
        // Serial.println(F("-----"));
        // Serial.println(F("Typ:    RPS"));
        // Serial.print(F("Eno-ID: "));
        // Serial.print(f_Pkt_st->u8DataBuffer[2], HEX);
        // Serial.print("-");
        // Serial.print(f_Pkt_st->u8DataBuffer[3], HEX);
        // Serial.print("-");
        // Serial.print(f_Pkt_st->u8DataBuffer[4], HEX);
        // Serial.print("-");
        // Serial.println(f_Pkt_st->u8DataBuffer[5], HEX);
        // Serial.print(F("Data:   "));
        // Serial.println(f_Pkt_st->u8DataBuffer[1], HEX);
        // Serial.println(F("-----"));
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_VLD) {
        Serial.println(F("-----"));
        Serial.println(F("Typ:    VLD"));
        Serial.print(F("Eno-ID: "));
        Serial.print(f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 5], HEX);
        Serial.print("-");
        Serial.print(f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 4], HEX);
        Serial.print("-");
        Serial.print(f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 3], HEX);
        Serial.print("-");
        Serial.println(f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 2], HEX);
        Serial.print(F("Data:  "));
        for (int i = 0; i < f_Pkt_st->u16DataLength - 6; i++) {
          Serial.print(" ");
          Serial.print(f_Pkt_st->u8DataBuffer[i + 1], HEX);
        }
        Serial.println(F(" "));
        Serial.println(F("-----"));
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_4BS) {
        Serial.println(F("-----"));
        Serial.println(F("Typ:    4BS"));
        Serial.print(F("Eno-ID: "));
        Serial.print(f_Pkt_st->u8DataBuffer[5], HEX);
        Serial.print("-");
        Serial.print(f_Pkt_st->u8DataBuffer[6], HEX);
        Serial.print("-");
        Serial.print(f_Pkt_st->u8DataBuffer[7], HEX);
        Serial.print("-");
        Serial.println(f_Pkt_st->u8DataBuffer[8], HEX);
        Serial.print(F("Data:   "));
        Serial.print(f_Pkt_st->u8DataBuffer[1], HEX);
        Serial.print(" ");
        Serial.print(f_Pkt_st->u8DataBuffer[2], HEX);
        Serial.print(" ");
        Serial.print(f_Pkt_st->u8DataBuffer[3], HEX);
        Serial.print(" ");
        Serial.println(f_Pkt_st->u8DataBuffer[4], HEX);
        Serial.println(F("-----"));
      } else if (f_Pkt_st->u8DataBuffer[0] == u8RORG_1BS) {
        Serial.println(F("-----"));
        Serial.println(F("Typ:    1BS"));
        Serial.print(F("Eno-ID: "));
        Serial.print(f_Pkt_st->u8DataBuffer[2], HEX);
        Serial.print("-");
        Serial.print(f_Pkt_st->u8DataBuffer[3], HEX);
        Serial.print("-");
        Serial.print(f_Pkt_st->u8DataBuffer[4], HEX);
        Serial.print("-");
        Serial.println(f_Pkt_st->u8DataBuffer[5], HEX);
        Serial.print(F("Data:   "));
        Serial.println(f_Pkt_st->u8DataBuffer[1], HEX);
        Serial.println(F("-----"));
      }
#endif

      bool packetWasHandled = false;
      for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
        if (_channels[i]->check_Eno_ID(f_Pkt_st)) {
          packetWasHandled = true;
        }
      }

#ifdef KDEBUG_handled
      if (!packetWasHandled) {
        Serial.println(F("Data not handled!"));
      } else {
        Serial.println(F("Data handled :-)"));
      }
#endif
    }
  }
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

    uart_sendPacket(&l_TestPacket_st);
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

    uart_sendPacket(&l_TestPacket_st);
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

    if (!uart_sendPacket(&lRdBaseIDPkt_st))
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

    if (!uart_sendPacket(&lRdBaseIDPkt_st))
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

    if (!uart_sendPacket(&lRdBaseIDPkt_st))
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

    if (!uart_sendPacket(&lRdBaseIDPkt_st))
    {
        logInfoP("Sending telegram failed.");
    }
    else
    {
        logInfoP("Requested Meas Value");
    }
}


void EnoceanModule::setRepeaterFunc()
{
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf[3];
  // uint8_t loopCount = 0;

  lu8SndBuf[0] = u8CO_WR_REPEATER;
  lu8SndBuf[1] = ParamENO_RepeaterFunc; //(knx.paramByte(ENO_RepeaterFunc) >> ENO_RepeaterFuncShift) & 1;
  lu8SndBuf[2] = ParamENO_RepeaterLevel; //knx.paramByte(ENO_RepeaterLevel);

  lRdBaseIDPkt_st.u16DataLength = 0x0003;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (Repeater ON/OFF).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st))
  {
    u8RetVal = ENOCEAN_NO_RX_TEL;
    logInfoP("Receiving telegram (Repeater ON/OFF).");
    while (u8RetVal == ENOCEAN_NO_RX_TEL)
    {
      u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
    }

    switch (u8RetVal)
    {
    case ENOCEAN_OK:
    {
      {
        std::string hexStr;
        char hexByte[4];
        for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++)
        {
          sprintf(hexByte, "%02X ", m_Pkt_st.u8DataBuffer[i]);
          hexStr += hexByte;
        }
        logInfoP("Data: %s", hexStr.c_str());
      }

      switch (m_Pkt_st.u8Type)
      {
      case u8RESPONSE:
      {
        switch (m_Pkt_st.u8DataBuffer[0])
        {
        case 0x00:
          logInfoP("Received Response = OK");
          break;
        case 0x02:
          logInfoP("Received Response = RET_NOT_SUPPORTED");
          break;
        case 0x03:
          logInfoP("Received Response = RET_WRONG_PARAM");
          break;
        default:
          break;
        }
      }
      break;
      default:
      {
        logInfoP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      }
    }
    break;
    case ENOCEAN_NO_RX_TEL:
      logInfoP("ERROR Receiving telegram (read base ID).");
      break;
    default:
    {
      logInfoP("Error receiving base ID");
    }
    }
  }
}

void EnoceanModule::readRepeaterFunc()
{
  PACKET_SERIAL_TYPE_ lRdBaseIDPkt_st;

  uint8_t lu8SndBuf[1];
  // uint8_t loopCount = 0;

  lu8SndBuf[0] = u8CO_RD_REPEATER;

  lRdBaseIDPkt_st.u16DataLength = 0x0001;
  lRdBaseIDPkt_st.u8OptionLength = 0x00;
  lRdBaseIDPkt_st.u8Type = u8RORG_COMMON_COMMAND;
  lRdBaseIDPkt_st.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (Repeater ON/OFF).");

  if (ENOCEAN_OK == uart_sendPacket(&lRdBaseIDPkt_st))
  {
    u8RetVal = ENOCEAN_NO_RX_TEL;
    logInfoP("Receiving telegram (Repeater ON/OFF).");
    while (u8RetVal == ENOCEAN_NO_RX_TEL)
    {
      u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
    }

    switch (u8RetVal)
    {
    case ENOCEAN_OK:
    {
      {
        std::string hexStr;
        char hexByte[4];
        for (int i = 0; i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength; i++)
        {
          sprintf(hexByte, "%02X ", m_Pkt_st.u8DataBuffer[i]);
          hexStr += hexByte;
        }
        logInfoP("Data: %s", hexStr.c_str());
      }

      switch (m_Pkt_st.u8Type)
      {
      case u8RESPONSE:
      {
        if (m_Pkt_st.u8DataBuffer[0] == 0x00)
        {
          logInfoP("Received Response = OK");
          switch (m_Pkt_st.u8DataBuffer[1])
          {
          case 0x00:
            logInfoP("Repeater = OFF");
            break;
          case 0x01:
            logInfoP("Repeater = ON");
            break;
          case 0x02:
            logInfoP("Repeater = Seletive");
            break;
          default:
            break;
          }
          switch (m_Pkt_st.u8DataBuffer[2])
          {
          case 0x01:
            logInfoP("Repeater = Level-1");
            break;
          case 0x02:
            logInfoP("Repeater = Level-2");
            break;
          default:
            break;
          }
        }
      }
      break;
      default:
      {
        logInfoP("Wrong packet type. Expected response. Received: %u", m_Pkt_st.u8Type);
      }
      }
    }
    break;
    case ENOCEAN_NO_RX_TEL:
      logInfoP("ERROR Receiving telegram (read base ID).");
      break;
    default:
    {
      logInfoP("Error receiving base ID");
    }
    }
  }
}

EnoceanModule openknxEnoceanModule;
