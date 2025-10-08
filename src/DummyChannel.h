#pragma once
#include "OpenKNX.h"

class EnoceanChannel : public OpenKNX::Channel
{
  private:

  public:
    EnoceanChannel(uint8_t index);
    const std::string name() override;
    void setup() override;
    void loop() override;

};