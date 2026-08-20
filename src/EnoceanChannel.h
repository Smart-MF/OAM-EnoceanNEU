#pragma once
#include "OpenKNX.h"

// Full definition lives in EnoceanModule.h; only a pointer is needed here,
// so a forward declaration avoids a circular include (EnoceanModule.h
// already includes EnoceanChannel.h).
struct PACKET_SERIAL_TYPE_;

class EnoceanChannel : public OpenKNX::Channel
{
  private:

  uint8_t deviceId_Arr[4] = {0, 0, 0, 0};

  public:
    EnoceanChannel(uint8_t index);
    const std::string name() override;
    void setup() override;
    void loop() override;
    bool check_Eno_ID(PACKET_SERIAL_TYPE_ *pPacket);
    void handleKnxEvent(int koIndex, GroupObject &ko);

};