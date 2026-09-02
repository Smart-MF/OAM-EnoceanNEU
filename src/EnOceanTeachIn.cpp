/*
 * EnOceanTeachIn.cpp
 * Siehe EnOceanTeachIn.h.
 */

#include "EnOceanTeachIn.h"

// CO_RD_SECUREDEVICES, Spec Kap. 1.10.29.
int EnOceanTeachIn::readSecureDevices(EnOceanSecureDevice *out, int maxDevices) {
  uint8_t slf[MAX_MAPPED_DEVICES];
  uint32_t id[MAX_MAPPED_DEVICES];
  int count = _esp3.readSecureDevices(slf, id, maxDevices < MAX_MAPPED_DEVICES ? maxDevices : MAX_MAPPED_DEVICES);
  if (count < 0)
    return -1;

  for (int i = 0; i < count && i < maxDevices; i++) {
    out[i].slf = slf[i];
    out[i].id = id[i];
  }
  return count;
}

void EnOceanTeachIn::rememberChannel(uint32_t id, uint8_t channel) {
  if (_mapCount >= MAX_MAPPED_DEVICES)
    return;
  _map[_mapCount].id = id;
  _map[_mapCount].channel = channel;
  _mapCount++;
  // Fuer reset-feste Speicherung hier zusaetzlich in Flash/EEPROM schreiben - das ist Teil eurer Anwendung,
  // nicht dieser Lib.
}

// Pollt den ESP3-Treiber waehrend des Lernfensters nicht-blockierend weiter (EnOceanESP3::poll() teilt sich die
// Empfangs-Statemachine mit EnoceanModule::task(); ausserhalb eines Lernfensters aufgerufen wuerde diese Funktion
// also nichts Falsches tun - es ist derselbe Mechanismus, nur hier in einer blockierenden Warteschleife benutzt),
// und meldet CO_EVENT_SECUREDEVICES-Events (Spec Kap. 1.9.7) an den optionalen Callback.
void EnOceanTeachIn::pumpEventsDuringWindow(uint32_t windowMs) {
  uint32_t start = millis();
  while ((millis() - start) < windowMs) {
    uint8_t ret = _esp3.poll();
    if (ret != ENOCEAN_OK) {
      // Kein volles Paket in diesem Tick: kurz abgeben statt die CPU (und damit z.B. den Watchdog oder den
      // zweiten Core) mit einer reinen Busy-Wait-Schleife fuer die ganze Fensterdauer zu blockieren.
      delay(1);
      continue;
    }

    PACKET_SERIAL_TYPE_ &pkt = _esp3.packet();
    if (pkt.u8Type == u8EVENT && pkt.u8DataBuffer[0] == u8EVT_CO_EVENT_SECUREDEVICES) {
      uint8_t cause = pkt.u8DataBuffer[1];
      uint32_t id = ((uint32_t)pkt.u8DataBuffer[2] << 24) | ((uint32_t)pkt.u8DataBuffer[3] << 16) |
                    ((uint32_t)pkt.u8DataBuffer[4] << 8) | (uint32_t)pkt.u8DataBuffer[5];
      if (_eventCb)
        _eventCb(id, cause);
    }
    // RADIO-Pakete (Type 1) laufen hier ebenfalls durch, werden aber fuer die eigentliche Teach-In-Logik nicht
    // benoetigt - das Entschluesseln/Verifizieren des Secure-Telegramms passiert intern im TCM515.
  }
}

bool EnOceanTeachIn::teachInOnChannel(uint8_t channel, uint32_t windowMs, uint32_t *newDeviceIdOut) {
  EnOceanSecureDevice before[MAX_MAPPED_DEVICES];
  int beforeN = readSecureDevices(before, MAX_MAPPED_DEVICES);
  if (beforeN < 0)
    beforeN = 0;

  if (!_esp3.activateLearnMode(channel, true, windowMs)) {
    return false;
  }

  pumpEventsDuringWindow(windowMs);

  _esp3.activateLearnMode(channel, false);

  EnOceanSecureDevice after[MAX_MAPPED_DEVICES];
  int afterN = readSecureDevices(after, MAX_MAPPED_DEVICES);
  if (afterN < 0)
    afterN = 0;

  for (int i = 0; i < afterN; i++) {
    bool wasThere = false;
    for (int j = 0; j < beforeN; j++) {
      if (before[j].id == after[i].id) {
        wasThere = true;
        break;
      }
    }
    if (!wasThere) {
      rememberChannel(after[i].id, channel);
      if (newDeviceIdOut)
        *newDeviceIdOut = after[i].id;
      return true;
    }
  }
  return false;
}
