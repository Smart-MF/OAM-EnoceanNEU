#pragma once
#include "OpenKNX.h"


#define CH_inaktive        0
#define EEP_Profil         1
#define Hersteller_Eltako  2

// ETS-Parameter "Einrichtung des Geräts" (CHSetupMode)
#define ENO_SETUP_MANUAL   0
#define ENO_SETUP_TEACHIN  1




// Full definition lives in EnoceanModule.h; only a pointer is needed here,
// so a forward declaration avoids a circular include (EnoceanModule.h
// already includes EnoceanChannel.h).
struct PACKET_SERIAL_TYPE_;

// Hier definiert (statt in EnoceanModule.h), da diese Datei EnoceanModule.h wegen des zirkulären Includes nicht einbinden kann.
#define RockerIdle 0x01

// Taster-Emulation KNX -> EnOcean: Dauer eines kurzen Tastendrucks und maximale Haltedauer beim Dimmen (ms)
#define ROCKER_TAP_MS 150
#define ROCKER_HOLD_MAX_MS 10000

union EnoceanHandle {
  uint8_t msg_sent_after_receive;           // Für MVA-005  A5-20-06
  uint8_t rockerState_pressed = RockerIdle; // ROCKER
};

// Definition liegt in EnoceanChannel.cpp, um Mehrfachdefinitionen (ODR-Verletzung) zu vermeiden,
// da diese Header-Datei in mehrere .cpp-Dateien eingebunden wird.
extern EnoceanHandle unionMSG;

class EnoceanChannel : public OpenKNX::Channel {
private:
  // Zustand des Tasters (Kanal-Profil Rocker): gedrückte logische Wippe (0xFF = keine), Seite, Zeitpunkt, langer Druck
  uint8_t _rockerWippe = 0xFF;
  bool _rockerUpper = false;
  bool _rockerLong = false;
  uint32_t _rockerPressTime = 0;

  // Taster-Emulation KNX -> EnOcean: Zeitpunkt für das Loslass-Telegramm (0 = keines ausstehend) und zuletzt
  // gesendeter Tastencode (für Jalousie-Stopp)
  uint32_t _knxRockerReleaseAt = 0;
  uint8_t _knxRockerLastDb0 = 0;

  // Ausfallüberwachung: Zeitpunkt des letzten Telegramms des Geräts und ob der Ausfall bereits gemeldet wurde
  uint32_t _lastSeen = 0;
  bool _failed = false;

  void handleRocker(PACKET_SERIAL_TYPE_ *pPacket);
  void knxRockerPress(uint8_t db0, uint32_t releaseAfterMs);
  void deviceSeen();
  void checkFailure();

public:
  EnoceanChannel(uint8_t index);
  const std::string name() override;
  void setup() override;
  void loop() override;
  bool check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket);
  bool matchesDeviceId(const uint8_t id[4]) const;
  void processKnxRocker(GroupObject &ko);
  bool sendLearnTelegram();
};