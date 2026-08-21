#pragma once
#include "OpenKNX.h"


#define CH_inaktive        0
#define EEP_Profil         1
#define Hersteller_Eltako  2




// Full definition lives in EnoceanModule.h; only a pointer is needed here,
// so a forward declaration avoids a circular include (EnoceanModule.h
// already includes EnoceanChannel.h).
struct PACKET_SERIAL_TYPE_;

// Hier definiert (statt in EnoceanModule.h), da diese Datei EnoceanModule.h wegen des zirkulären Includes nicht einbinden kann.
#define RockerIdle 0x01

union EnoceanHandle {
  uint8_t msg_sent_after_receive;           // Für MVA-005  A5-20-06
  uint8_t rockerState_pressed = RockerIdle; // ROCKER
};

// Definition liegt in EnoceanChannel.cpp, um Mehrfachdefinitionen (ODR-Verletzung) zu vermeiden,
// da diese Header-Datei in mehrere .cpp-Dateien eingebunden wird.
extern EnoceanHandle unionMSG;

class EnoceanChannel : public OpenKNX::Channel {
private:
  uint8_t deviceId_Arr[4] = {0, 0, 0, 0};

public:
  EnoceanChannel(uint8_t index);
  const std::string name() override;
  void setup() override;
  void loop() override;
  bool check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket);
};