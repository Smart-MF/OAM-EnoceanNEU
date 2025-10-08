#include "EnoceanChannel.h"
#include "Arduino.h"

EnoceanChannel::EnoceanChannel(uint8_t index)
{
    _channelIndex = index;
}

const std::string EnoceanChannel::name()
{
    return "EnoceanChannel";
}

void EnoceanChannel::setup()
{
    logInfoP("setup");
    logIndentUp();
    logDebugP("debug setup");
    logTraceP("trace setup");
    logIndentDown();
}

void EnoceanChannel::loop()
{
  // Dummy Action
  delayMicroseconds(100);
}