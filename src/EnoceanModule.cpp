#include "EnoceanModule.h"

#define proc_crc8(u8CRC, u8Data) (u8CRC8Table[u8CRC ^ u8Data])

uint8_t EnoceanModule::u8CRC8Table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
    0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
    0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
    0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
    0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
    0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
    0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
    0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
    0x6A, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
    0xfa, 0xfd, 0xf4, 0xf3};

EnoceanModule::EnoceanModule() {}

const std::string EnoceanModule::name() { return "Enocean"; }

const std::string EnoceanModule::version() {
  // hides the module in the version output on the console, because the firmware
  // version is sufficient.
  return "";
}

void EnoceanModule::initSerial(Stream &serial) { _serial = &serial; }

void EnoceanModule::setup(bool configured) {
  delay(2000);
  logInfoP("Setup0");
  logIndentUp();

  OPENKNX_ENO_SERIAL.setRX(OPENKNX_ENO_UART_RX_PIN);
  OPENKNX_ENO_SERIAL.setTX(OPENKNX_ENO_UART_TX_PIN);

  OPENKNX_ENO_SERIAL.begin(57600);
  initSerial(OPENKNX_ENO_SERIAL);

  if (configured) {
    // setupCustomFlash();
    setupChannels();
  }
  delay(2000);
  logInfoP("Begin");
  begin();

  logIndentDown();
}

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

void EnoceanModule::loop(bool configured) {
  if (delayCheck(_timer1, 5000)) {
     logInfoP("Loop0");
    _timer1 = millis();
  }

  if (configured) {
    if (ParamENO_VisibleChannels == 0)
      return;

    uint8_t processed = 0;
    do {
      task();

      _channels[_currentChannel]->loop();
    } while (openknx.freeLoopIterate(ParamENO_VisibleChannels, _currentChannel,
                                     processed));
  }
}

#ifdef OPENKNX_DUALCORE

void EnoceanModule::setup1(bool configured) {
  delay(1000);
  // logInfoP("Setup1");
}

void EnoceanModule::loop1(bool configured) {
  if (delayCheck(_timer2, 7200)) {
    // logInfoP("Loop1");
    _timer2 = millis();
  }
}
#endif

void EnoceanModule::task() {
  static uint32_t lastTime = 0;
  if (millis() - lastTime > 5000) {
    logInfoP("Alive, runtime: %u",lastTime);
    lastTime = millis();
  }

  // EnOcean IN -> KNX OUT
  u8RetVal = ENOCEAN_NO_RX_TEL;
  // u8RetVal = uart_getPacket(&m_Pkt_st, (uint16_t)DATBUF_SZ);
  getEnOceanMSG(u8RetVal, &m_Pkt_st);
}

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

  /*
  // 2.) compare old base-ID with the new ID
  if ((knx.paramByte(ENO_SetBaseIdFunc) >> ENO_SetBaseIdFuncShift) & 1)
  {
    if (checkBaseID())
    { // old != new
#ifdef KDEBUG
      logDebugP("Base-ID: OLD != NEW -> change! ");
#endif
      setBaseId(&lui8_BaseID_p[0]);
      // 3.) read Base-ID again and print it out
      readBaseId(&lui8_BaseID_p[0]);
    }
    else // old == new
    {
#ifdef KDEBUG
      logDebugP("Base-ID: OLD == NEW -> NO change! ");
#endif
    }
  }
 */

  for (int i = 0; i < BASEID_BYTES; i++) {
    logDebugP("Base-ID: %i", lui8_BaseID_p[i], HEX);
  }

  //****************** Repeater Function ************************************
  /*
  #ifdef KDEBUG
    logDebugP("----------------------");
  #endif
    setRepeaterFunc();
    // prüft ob Änderungen umgesetzt wurden
    readRepeaterFunc();

  #ifdef KDEBUG
    logDebugP("----------------------");
  #endif
  */

  isInited = true;
}

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
      for (int i = 0;
           i < m_Pkt_st.u16DataLength + (uint16_t)m_Pkt_st.u8OptionLength;
           i++) {
        logDebugP("Data: %u", m_Pkt_st.u8DataBuffer[i], HEX);
      }
      switch (m_Pkt_st.u8Type) {
      case u8RESPONSE:
        logDebugP("Received Response.");
        for (int i = 0; i < BASEID_BYTES; i++) {
          memcpy((void *)&(fui8_BaseID_p[i]),
                 (void *)&(m_Pkt_st.u8DataBuffer[i + 1]), 1);
        }
        break;
      default:
        logDebugP("Wrong packet type. Expected response. Received: %u",
                  m_Pkt_st.u8Type);
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

void EnoceanModule::processInputKo(GroupObject &ko) {
  // logDebugP("processInputKo GA%04X", ko.asap());
  // logHexDebugP(ko.valueRef(), ko.valueSize());
}

void EnoceanModule::showHelp() {
  openknx.console.printHelpLine("dummy", "Print a dummy text");
}

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
void EnoceanModule::registerUsbExchangeCallbacks() {
  // Sample
  openknxUsbExchangeModule.onLoad(
      "Dummy.txt", [](UsbExchangeFile *file) -> void { file->write("Demo"); });
  openknxUsbExchangeModule.onEject(
      "Dummy.txt", [](UsbExchangeFile *file) -> bool {
        // File is required
        if (file == nullptr) {
          logError("EnoceanModule",
                   "File Dummy.txt was deleted but is mandatory");
          return false;
        }
        return true;
      });
}
#endif
#endif

uint8_t EnoceanModule::uart_getPacket(PACKET_SERIAL_TYPE_ *pPacket,
                                      uint16_t u16BufferLength) {

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

void EnoceanModule::getEnOceanMSG(uint8_t u8RetVal,
                                  PACKET_SERIAL_TYPE_ *f_Pkt_st) {
  if (u8RetVal == ENOCEAN_OK) {
#ifdef KDEBUG_Received
    Serial.print(F("Received Data: "));
    for (int i = 0;
         i < f_Pkt_st->u16DataLength + (uint16_t)f_Pkt_st->u8OptionLength;
         i++) {
      // Serial.print(F("%X"), f_Pkt_st->u8DataBuffer[i]);
      Serial.print(f_Pkt_st->u8DataBuffer[i], HEX);
      Serial.print(F(" "));
    }
    Serial.println(F(""));
#endif

    if (f_Pkt_st->u8Type == u8RADIO_ERP1) {

#ifdef KDEBUG_min
      if (f_Pkt_st->u8DataBuffer[0] == u8RORG_RPS) {
        logDebugP("RPS: Data:%u ID:%u",f_Pkt_st->u8DataBuffer[1], f_Pkt_st->u8DataBuffer[2],f_Pkt_st->u8DataBuffer[3],f_Pkt_st->u8DataBuffer[4], f_Pkt_st->u8DataBuffer[5], HEX, HEX,HEX,HEX,HEX)
        //Serial.println(F("-----"));
        //Serial.println(F("Typ:    RPS"));
        //Serial.print(F("Eno-ID: "));
        //Serial.print(f_Pkt_st->u8DataBuffer[2], HEX);
        //Serial.print("-");
        //Serial.print(f_Pkt_st->u8DataBuffer[3], HEX);
        //Serial.print("-");
        //Serial.print(f_Pkt_st->u8DataBuffer[4], HEX);
        //Serial.print("-");
        //Serial.println(f_Pkt_st->u8DataBuffer[5], HEX);
        //Serial.print(F("Data:   "));
        //Serial.println(f_Pkt_st->u8DataBuffer[1], HEX);
        //Serial.println(F("-----"));
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
        Serial.println(f_Pkt_st->u8DataBuffer[f_Pkt_st->u16DataLength - 2],
                       HEX);
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

      /*
            bool packetWasHandled = false;
            for (int i = 0; i < lastDevice; ++i)
            {
              if (deviceRegistry[i]->handleEnOceanPacket(f_Pkt_st))
              {
                packetWasHandled = true;
              }
            }

      #ifdef KDEBUG_handled
            if (!packetWasHandled)
            {
              Serial.println(F("Data not handled!"));
            }
            else
            {
              Serial.println(F("Data handled :-)"));
            }
      #endif
      */
    }
  }
}

EnoceanModule openknxEnoceanModule;