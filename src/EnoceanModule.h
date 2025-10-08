#pragma once
#include "DummyChannel.h"
#include "OpenKNX.h"
#ifdef ARDUINO_ARCH_RP2040
    #ifndef OPENKNX_USB_EXCHANGE_IGNORE
        #include "UsbExchangeModule.h"
    #endif
#endif

class EnoceanModule : public OpenKNX::Module
{
  private:
    uint32_t _timer1 = 0;
    uint32_t _timer2 = 0;
    uint8_t _currentChannel = 0;
    Stream *_serial;
    EnoceanChannel *_channels[ENO_ChannelCount];
    OpenKNX::Flash::Driver *_dummyStorage = nullptr;

    void setupCustomFlash();
    void setupChannels();
    void initSerial(Stream &serial);
    void readBaseId(uint8_t *fui8_BaseID_p);
#ifdef ARDUINO_ARCH_RP2040
    #ifndef OPENKNX_USB_EXCHANGE_IGNORE
    void registerUsbExchangeCallbacks();
    #endif
#endif

  public:
    EnoceanModule();
    void loop(bool configured) override;
    void setup(bool configured) override;
#ifdef OPENKNX_DUALCORE
    void loop1(bool configured) override;
    void setup1(bool configured) override;
#endif
    const std::string name() override;
    const std::string version() override;
    void processInputKo(GroupObject &ko) override;
    bool processCommand(const std::string cmd, bool diagnoseKo);
    void showHelp() override;
};

extern EnoceanModule openknxEnoceanModule;