#include "EnoceanModule.h"

EnoceanModule::EnoceanModule() {}

const std::string EnoceanModule::name() { return "Enocean"; }

const std::string EnoceanModule::version() {
  // hides the module in the version output on the console, because the firmware
  // version is sufficient.
  return "";
}

void EnoceanModule::initSerial(Stream &serial) { _serial = &serial; }

void EnoceanModule::setup(bool configured) {
  // delay(1000);
  logInfoP("Setup0");
  logIndentUp();

  OPENKNX_ENO_SERIAL.setRX(OPENKNX_ENO_UART_RX_PIN);
  OPENKNX_ENO_SERIAL.setTX(OPENKNX_ENO_UART_TX_PIN);

  OPENKNX_ENO_SERIAL.begin(57600);

  if (configured) {
    // setupCustomFlash();
    setupChannels();
  }
  logIndentDown();
}

void EnoceanModule::setupChannels() {
  for (uint8_t i = 0; i < ParamENO_VisibleChannels; i++) {
    _channels[i] = new EnoceanChannel(i);
    _channels[i]->setup();
  }
}

void EnoceanModule::setupCustomFlash() {
  logDebugP("initialize Enocean flash");
  OpenKNX::Flash::Driver _EnoceanStorage;
#ifdef ARDUINO_ARCH_ESP32
  _EnoceanStorage.init("Enocean");
#else
  _EnoceanStorage.init("Enocean", Enocean_FLASH_OFFSET, Enocean_FLASH_SIZE);
#endif

  logTraceP("write Enocean data");
  // _EnoceanStorage.writeByte(0, 0x11);
  // _EnoceanStorage.writeWord(1, 0xFFFF);
  // _EnoceanStorage.writeInt(3, 6666666);
  // for (size_t i = 0; i < 4095; i++)
  // {
  //     _EnoceanStorage.writeByte(i, i);
  // }
  // _EnoceanStorage.commit();

  logDebugP("read Enocean data");
  logIndentUp();
  // logHexDebugP(_EnoceanStorage.flashAddress(), 4095);
  // logDebugP("byte: %02X", _EnoceanStorage.readByte(0)); // UINT8
  // logDebugP("word: %i", _EnoceanStorage.readWord(1));   // UINT16
  // logDebugP("int: %i", _EnoceanStorage.readInt(3));     // UINT32

  logIndentDown();
}

void EnoceanModule::loop(bool configured) {
  if (delayCheck(_timer1, 5100)) {
    // logInfoP("Loop0");
    _timer1 = millis();
  }

  if (configured) {
    if (ParamENO_VisibleChannels == 0)
      return;

    uint8_t processed = 0;
    do
      _channels[_currentChannel]->loop();

    while (openknx.freeLoopIterate(ParamENO_VisibleChannels, _currentChannel,
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

void ENOModule::begin() {
  if (isInited)
    return;

  m_Pkt_st.u8DataBuffer = &u8datBuf[0];

  //****************** Read, Check & Set EnOcean Gateway Base ID
  //************************************
  // communicates via Enocean UART channel
  // 1.) read base-ID

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

EnoceanModule openknxEnoceanModule;