/*
 * EnOceanESP3.cpp
 *
 * Siehe EnOceanESP3.h. CRC8-Tabelle 1:1 aus der EnOcean-Spec V1.21,
 * Kap. 2.3 uebernommen (Polynom G(x) = x^8 + x^2 + x^1 + x^0).
 *
 * poll()/sendPacket() sind funktional identisch zum bisherigen
 * EnoceanModule::uart_getPacket()/uart_sendPacket() (Referenz-C-Code der
 * Spec, Kap. 2.4.2/2.4.3), bauen Header/CRC hier aber ueber einen lokalen
 * 4-Byte-Puffer auf statt (wie zuvor) das PACKET_SERIAL_TYPE_-Struct per
 * reinterpret_cast als rohen Byte-Puffer zu behandeln - das war zwar durch
 * die gleiche Idee aus dem Referenzcode motiviert, ist aber wegen des
 * Pointer-Members in der Struct auf Dauer fragil.
 */

#include "EnOceanESP3.h"

uint8_t EnOceanESP3::u8CRC8Table[256] = {
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

std::string EnOceanESP3::logPrefix() { return openknx.logger.buildPrefix("EnOceanESP3", 0); }

uint8_t EnOceanESP3::crc8(uint8_t crc, uint8_t dataByte) { return u8CRC8Table[crc ^ dataByte]; }

// Merkt sich den (bereits initialisierten) UART-Stream und initialisiert die Empfangs-Statemachine.
void EnOceanESP3::begin(Stream &serial) {
  _serial = &serial;
  _rxPacket.u8DataBuffer = _rxBuffer;
  _rxState = GET_SYNC_STATE;
  _rxCount = 0;
  _rxCrc = 0;
}

// Sendet ein Paket im ESP3-Format (Sync, Header, CRC8H, Data+OptionalData, CRC8D), Spec Kap. 1.3/2.4.3.
uint8_t EnOceanESP3::sendPacket(PACKET_SERIAL_TYPE_ *pPacket) {
  if (_serial == nullptr)
    return ENOCEAN_OUT_OF_RANGE;

  // Wenn beide Laengenfelder 0 sind, ist das Telegramm nicht zulaessig.
  if ((pPacket->u16DataLength || pPacket->u8OptionLength) == 0)
    return ENOCEAN_OUT_OF_RANGE;

  uint8_t header[SER_HEADER_NR_BYTES];
  header[0] = (uint8_t)(pPacket->u16DataLength >> 8); // MSB first, Spec Kap. 1.6.1
  header[1] = (uint8_t)(pPacket->u16DataLength & 0xFF);
  header[2] = pPacket->u8OptionLength;
  header[3] = pPacket->u8Type;

  uint8_t crc8h = 0;
  for (uint8_t i = 0; i < SER_HEADER_NR_BYTES; i++)
    crc8h = crc8(crc8h, header[i]);

  uint16_t payloadLen = pPacket->u16DataLength + (uint16_t)pPacket->u8OptionLength;
  uint8_t crc8d = 0;
  for (uint16_t i = 0; i < payloadLen; i++)
    crc8d = crc8(crc8d, pPacket->u8DataBuffer[i]);

  while (_serial->write(SER_SYNCH_CODE) != 1)
    ;
  _serial->write(header, SER_HEADER_NR_BYTES);
  while (_serial->write(crc8h) != 1)
    ;
  for (uint16_t i = 0; i < payloadLen; i++) {
    while (_serial->write(pPacket->u8DataBuffer[i]) != 1)
      ;
  }
  while (_serial->write(crc8d) != 1)
    ;

  return ENOCEAN_OK;
}

// Nicht-blockierende Empfangs-Statemachine: verarbeitet alle aktuell im UART-Puffer wartenden Bytes,
// prueft Sync/CRC8H/CRC8D und liefert ein vollstaendiges Paket (in packet()) oder einen Statuscode.
// Referenz: Spec Kap. 2.4.2.
uint8_t EnOceanESP3::poll() {
  if (_serial == nullptr)
    return ENOCEAN_NO_RX_TEL;

  while (_serial->available() > 0) {
    uint8_t rxByte;
    if (_serial->readBytes(&rxByte, 1) != 1)
      break;

    switch (_rxState) {
    case GET_SYNC_STATE:
      if (rxByte == SER_SYNCH_CODE) {
        _rxState = GET_HEADER_STATE;
        _rxCount = 0;
        _rxCrc = 0;
      }
      break;

    case GET_HEADER_STATE:
      _rxHeader[_rxCount++] = rxByte;
      _rxCrc = crc8(_rxCrc, rxByte);
      if (_rxCount == SER_HEADER_NR_BYTES) {
        _rxState = CHECK_CRC8H_STATE;
      }
      break;

    case CHECK_CRC8H_STATE: {
      if (_rxCrc != rxByte) {
        // Header-CRC falsch: pruefen, ob im bisherigen Header (oder im gerade
        // empfangenen Byte selbst) ein Sync-Byte steckt, um daran neu zu
        // synchronisieren (Resync-Logik der Spec).
        int a = -1;
        for (int i = 0; i < SER_HEADER_NR_BYTES; i++) {
          if (_rxHeader[i] == SER_SYNCH_CODE) {
            a = i + 1;
            break;
          }
        }
        if (a == -1 && rxByte != SER_SYNCH_CODE) {
          _rxState = GET_SYNC_STATE;
          break;
        }
        if (a == -1 && rxByte == SER_SYNCH_CODE) {
          _rxState = GET_HEADER_STATE;
          _rxCount = 0;
          _rxCrc = 0;
          break;
        }
        // Header enthaelt ein Sync-Byte: Puffer ab dort verschieben und CRC neu berechnen.
        _rxCrc = 0;
        for (int i = 0; i < (SER_HEADER_NR_BYTES - a); i++) {
          _rxHeader[i] = _rxHeader[a + i];
          _rxCrc = crc8(_rxCrc, _rxHeader[i]);
        }
        _rxCount = SER_HEADER_NR_BYTES - a;
        _rxHeader[_rxCount++] = rxByte;
        _rxCrc = crc8(_rxCrc, rxByte);
        if (_rxCount < SER_HEADER_NR_BYTES) {
          _rxState = GET_HEADER_STATE;
        }
        // sonst: Zustand bleibt CHECK_CRC8H_STATE, das naechste Byte gilt als Kandidat fuer CRC8H.
        break;
      }

      // Header-CRC korrekt: Header-Felder dekodieren (Data Length MSB-first, Spec Kap. 1.6.1).
      _rxPacket.u16DataLength = ((uint16_t)_rxHeader[0] << 8) | _rxHeader[1];
      _rxPacket.u8OptionLength = _rxHeader[2];
      _rxPacket.u8Type = _rxHeader[3];

      if ((_rxPacket.u16DataLength + (uint16_t)_rxPacket.u8OptionLength) == 0) {
        if (rxByte == SER_SYNCH_CODE) {
          _rxState = GET_HEADER_STATE;
          _rxCount = 0;
          _rxCrc = 0;
          break;
        }
        // Paket mit korrektem CRC8H, aber ungueltigen Laengenfeldern.
        _rxState = GET_SYNC_STATE;
        return ENOCEAN_OUT_OF_RANGE;
      }

      _rxState = GET_DATA_STATE;
      _rxCount = 0;
      _rxCrc = 0;
      break;
    }

    case GET_DATA_STATE:
      if (_rxCount < DATBUF_SZ) {
        _rxPacket.u8DataBuffer[_rxCount] = rxByte;
        _rxCrc = crc8(_rxCrc, rxByte);
      }
      if (++_rxCount == (uint16_t)(_rxPacket.u16DataLength + (uint16_t)_rxPacket.u8OptionLength)) {
        _rxState = CHECK_CRC8D_STATE;
      }
      break;

    case CHECK_CRC8D_STATE:
      _rxState = GET_SYNC_STATE;
      if (_rxCount > DATBUF_SZ)
        return ENOCEAN_OUT_OF_RANGE;
      if (_rxCrc == rxByte)
        return ENOCEAN_OK;
      if (rxByte == SER_SYNCH_CODE) {
        _rxState = GET_HEADER_STATE;
        _rxCount = 0;
        _rxCrc = 0;
      }
      return ENOCEAN_NOT_VALID_CHKSUM;
    }
  }

  return (_rxState == GET_SYNC_STATE) ? ENOCEAN_NO_RX_TEL : ENOCEAN_NEW_RX_BYTE;
}

// Sendet ein COMMON_COMMAND-Requestpaket und wartet blockierend (max. timeoutMs) auf die zugehoerige
// RESPONSE; dabei asynchron eintreffende Fremdpakete (z.B. Funktelegramme) werden verworfen.
bool EnOceanESP3::sendAndWaitResponse(PACKET_SERIAL_TYPE_ *pRequest, uint32_t timeoutMs) {
  if (ENOCEAN_OK != sendPacket(pRequest))
    return false;

  uint32_t start = millis();
  uint8_t ret;
  do {
    ret = poll();
    if (ret == ENOCEAN_OK)
      return true;
    if (ret == ENOCEAN_NOT_VALID_CHKSUM || ret == ENOCEAN_OUT_OF_RANGE)
      return false; // definitiver Fehler, weiteres Warten bringt nichts
  } while ((millis() - start) < timeoutMs);

  return false; // Timeout
}

// CO_RD_IDBASE, Spec Kap. 1.10.10.
bool EnOceanESP3::readBaseId(uint8_t fui8_BaseID_p[BASEID_BYTES], uint32_t timeoutMs) {
  uint8_t lu8SndBuf = u8CO_RD_IDBASE;

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 0x0001;
  lReq.u8OptionLength = 0x00;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf;

  logDebugP("Sending telegram (read base ID).");
  if (!sendAndWaitResponse(&lReq, timeoutMs)) {
    logDebugP("ERROR receiving telegram (read base ID).");
    return false;
  }
  if (_rxPacket.u8Type != u8RESPONSE) {
    logDebugP("Wrong packet type. Expected response. Received: %u", _rxPacket.u8Type);
    return false;
  }
  logHexDebugP(_rxPacket.u8DataBuffer, _rxPacket.u16DataLength + (uint16_t)_rxPacket.u8OptionLength);
  if (_rxPacket.u8DataBuffer[0] != 0x00) { // RET_OK
    logDebugP("Read base ID Return Code: %u", _rxPacket.u8DataBuffer[0]);
    return false;
  }

  memcpy(fui8_BaseID_p, &_rxPacket.u8DataBuffer[1], BASEID_BYTES);
  return true;
}

// CO_WR_IDBASE, Spec Kap. 1.10.9. Liefert den ESP3-Return-Code der Antwort.
uint8_t EnOceanESP3::setBaseId(const uint8_t fui8_BaseID_p[BASEID_BYTES], uint32_t timeoutMs) {
  uint8_t lu8SndBuf[5];
  lu8SndBuf[0] = u8CO_WR_IDBASE;
  lu8SndBuf[1] = fui8_BaseID_p[0];
  lu8SndBuf[2] = fui8_BaseID_p[1];
  lu8SndBuf[3] = fui8_BaseID_p[2];
  lu8SndBuf[4] = fui8_BaseID_p[3];

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 0x0005;
  lReq.u8OptionLength = 0x00;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (set base ID).");
  if (!sendAndWaitResponse(&lReq, timeoutMs)) {
    logInfoP("ERROR receiving telegram (set base ID).");
    return ENOCEAN_NO_RX_TEL;
  }
  if (_rxPacket.u8Type != u8RESPONSE) {
    logInfoP("Wrong packet type. Expected response. Received: %u", _rxPacket.u8Type);
    return ENOCEAN_NOT_VALID_CHKSUM;
  }

  uint8_t retCode = _rxPacket.u8DataBuffer[0];
  switch (retCode) {
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
  return retCode;
}

// CO_WR_LEARNMODE, Spec Kap. 1.10.25. Erzwingt fuer absolute Kanaele (1..0xFD), dass nie mehr als ein Kanal
// gleichzeitig im Lernmodus ist: ein zuvor ueber diese Methode aktivierter, abweichender Kanal wird vor dem
// Aktivieren eines neuen Kanals immer zuerst deaktiviert.
bool EnOceanESP3::activateLearnMode(uint8_t channel, bool enable, uint32_t deviceTimeoutMs,
                                     uint32_t responseTimeoutMs) {
  bool isAbsoluteChannel = channel <= 0xFD;

  if (enable && isAbsoluteChannel && _activeLearnChannel != 0 && _activeLearnChannel != channel) {
    logDebugP("Deactivating previous learn channel %u before activating %u.", _activeLearnChannel, channel);
    sendLearnMode(_activeLearnChannel, false, 0, responseTimeoutMs);
    _activeLearnChannel = 0;
  }

  bool ok = sendLearnMode(channel, enable, deviceTimeoutMs, responseTimeoutMs);

  if (isAbsoluteChannel) {
    if (enable)
      _activeLearnChannel = ok ? channel : 0;
    else if (_activeLearnChannel == channel)
      _activeLearnChannel = 0;
  }

  return ok;
}

// Rohes CO_WR_LEARNMODE ohne die Ein-Kanal-Garantie von activateLearnMode() (siehe dort).
bool EnOceanESP3::sendLearnMode(uint8_t channel, bool enable, uint32_t deviceTimeoutMs, uint32_t responseTimeoutMs) {
  // Data: COMMAND Code (1) + Enable (1) + Timeout (4, big-endian) = 6 Bytes; Optional Data: Channel (1 Byte)
  uint8_t lu8SndBuf[7];
  lu8SndBuf[0] = u8CO_WR_LEARNMODE;
  lu8SndBuf[1] = enable ? 1 : 0; // Start Learn mode = 1, End Learn mode = 0
  lu8SndBuf[2] = (uint8_t)(deviceTimeoutMs >> 24);
  lu8SndBuf[3] = (uint8_t)(deviceTimeoutMs >> 16);
  lu8SndBuf[4] = (uint8_t)(deviceTimeoutMs >> 8);
  lu8SndBuf[5] = (uint8_t)(deviceTimeoutMs);
  lu8SndBuf[6] = channel; // 0..0xFD = Kanal absolut, 0xFE = vorheriger, 0xFF = naechster Kanal (relativ)

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 6;
  lReq.u8OptionLength = 1;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf[0];

  logDebugP("Sending telegram (learn mode %s), Channel: %u", enable ? "on" : "off", channel);
  if (!sendAndWaitResponse(&lReq, responseTimeoutMs)) {
    logDebugP("ERROR receiving telegram (learn mode %s).", enable ? "on" : "off");
    _lastLearnModeResult = LEARNMODE_RESULT_NO_RESPONSE;
    return false;
  }
  if (_rxPacket.u8Type != u8RESPONSE) {
    logDebugP("Wrong packet type. Expected response. Received: %u", _rxPacket.u8Type);
    _lastLearnModeResult = LEARNMODE_RESULT_NO_RESPONSE;
    return false;
  }

  logDebugP("Learn mode Return Code: %u", _rxPacket.u8DataBuffer[0]);
  bool ok = _rxPacket.u8DataBuffer[0] == 0x00; // RET_OK
  _lastLearnModeResult = ok ? LEARNMODE_RESULT_OK : LEARNMODE_RESULT_REJECTED;
  return ok;
}

// CO_RD_LEARNMODE, Spec Kap. 1.10.26.
bool EnOceanESP3::readLearnMode(uint8_t &enable, uint8_t &channel, uint32_t timeoutMs) {
  uint8_t lu8SndBuf = u8CO_RD_LEARNMODE;

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 0x0001;
  lReq.u8OptionLength = 0x00;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf;

  logDebugP("Sending telegram (read learn mode).");
  if (!sendAndWaitResponse(&lReq, timeoutMs)) {
    logDebugP("ERROR receiving telegram (read learn mode).");
    _lastLearnModeResult = LEARNMODE_RESULT_NO_RESPONSE;
    return false;
  }
  if (_rxPacket.u8Type != u8RESPONSE || _rxPacket.u8DataBuffer[0] != 0x00) { // RET_OK
    logDebugP("Read learn mode Return Code: %u", _rxPacket.u8DataBuffer[0]);
    _lastLearnModeResult = (_rxPacket.u8Type == u8RESPONSE) ? LEARNMODE_RESULT_REJECTED : LEARNMODE_RESULT_NO_RESPONSE;
    return false;
  }

  enable = _rxPacket.u8DataBuffer[1];
  channel = _rxPacket.u8DataBuffer[2]; // Optional Data: Channel
  logDebugP("Learn mode Enable: %u, Channel: %u", enable, channel);
  _lastLearnModeResult = LEARNMODE_RESULT_OK;
  return true;
}

// CO_WR_REPEATER, Spec Kap. 1.10.11.
bool EnOceanESP3::setRepeater(uint8_t enable, uint8_t level, uint32_t timeoutMs) {
  uint8_t lu8SndBuf[3];
  lu8SndBuf[0] = u8CO_WR_REPEATER;
  lu8SndBuf[1] = enable;
  lu8SndBuf[2] = level;

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 0x0003;
  lReq.u8OptionLength = 0x00;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf[0];

  logInfoP("Sending telegram (Repeater ON/OFF).");
  if (!sendAndWaitResponse(&lReq, timeoutMs)) {
    logInfoP("ERROR receiving telegram (Repeater ON/OFF).");
    return false;
  }
  if (_rxPacket.u8Type != u8RESPONSE) {
    logInfoP("Wrong packet type. Expected response. Received: %u", _rxPacket.u8Type);
    return false;
  }

  switch (_rxPacket.u8DataBuffer[0]) {
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
  return _rxPacket.u8DataBuffer[0] == 0x00; // RET_OK
}

// CO_RD_SECUREDEVICES, Spec Kap. 1.10.29.
int EnOceanESP3::readSecureDevices(uint8_t outSlf[], uint32_t outId[], int maxDevices, uint32_t timeoutMs) {
  uint8_t lu8SndBuf = u8CO_RD_SECUREDEVICES;

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 0x0001;
  lReq.u8OptionLength = 0x00;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf;

  logDebugP("Sending telegram (read secure devices).");
  if (!sendAndWaitResponse(&lReq, timeoutMs)) {
    logDebugP("ERROR receiving telegram (read secure devices).");
    return -1;
  }
  if (_rxPacket.u8Type != u8RESPONSE || _rxPacket.u8DataBuffer[0] != 0x00) { // RET_OK
    logDebugP("Read secure devices Return Code: %u", _rxPacket.u8DataBuffer[0]);
    return -1;
  }

  // Data: Return Code (1) + (SLF (1) + Device-ID (4)) je Geraet, Spec Tab. 58
  int count = (_rxPacket.u16DataLength - 1) / 5;
  for (int i = 0; i < count && i < maxDevices; i++) {
    const uint8_t *e = &_rxPacket.u8DataBuffer[1 + i * 5];
    outSlf[i] = e[0];
    outId[i] = ((uint32_t)e[1] << 24) | ((uint32_t)e[2] << 16) | ((uint32_t)e[3] << 8) | (uint32_t)e[4];
  }
  return count;
}

// CO_RD_REPEATER, Spec Kap. 1.10.12.
bool EnOceanESP3::readRepeater(uint8_t &enable, uint8_t &level, uint32_t timeoutMs) {
  uint8_t lu8SndBuf = u8CO_RD_REPEATER;

  PACKET_SERIAL_TYPE_ lReq;
  lReq.u16DataLength = 0x0001;
  lReq.u8OptionLength = 0x00;
  lReq.u8Type = u8RORG_COMMON_COMMAND;
  lReq.u8DataBuffer = &lu8SndBuf;

  logInfoP("Sending telegram (read Repeater).");
  if (!sendAndWaitResponse(&lReq, timeoutMs)) {
    logInfoP("ERROR receiving telegram (read Repeater).");
    return false;
  }
  if (_rxPacket.u8Type != u8RESPONSE || _rxPacket.u8DataBuffer[0] != 0x00) { // RET_OK
    logInfoP("Wrong/failed response reading Repeater state.");
    return false;
  }

  enable = _rxPacket.u8DataBuffer[1];
  level = _rxPacket.u8DataBuffer[2];

  switch (enable) {
  case 0x00:
    logInfoP("Repeater = OFF");
    break;
  case 0x01:
    logInfoP("Repeater = ON");
    break;
  case 0x02:
    logInfoP("Repeater = Selective");
    break;
  default:
    break;
  }
  switch (level) {
  case 0x01:
    logInfoP("Repeater = Level-1");
    break;
  case 0x02:
    logInfoP("Repeater = Level-2");
    break;
  default:
    break;
  }
  return true;
}
