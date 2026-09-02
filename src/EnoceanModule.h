#pragma once
#include "EnOceanESP3.h"
#include "EnOceanTeachIn.h"
#include "EnoceanChannel.h"
#include "OpenKNX.h"
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
#include "UsbExchangeModule.h"
#endif
#endif

/*
    EEP type
*/

// Receive
#define u8RORG_1BS 0xD5    // 213
#define u8RORG_RPS 0xF6    // 246
#define u8RORG_VLD 0xD2    // 210
#define u8RORG_4BS 0xA5    // 165
#define u8RORG_Rocker 0xFA // 250

// Send
#define u8KNX_RORG_Rocker 1
#define u8KNX_RORG_VLD 2
#define u8KNX_RORG_4BS 3

// ENOCEAN_*, u8RADIO_ERP1/u8RESPONSE, u8RORG_COMMON_COMMAND, u8CO_*,
// PACKET_SERIAL_TYPE_, STATES_GET_PACKET_, BASEID_BYTES, DATBUF_SZ,
// SER_SYNCH_CODE/SER_HEADER_NR_BYTES kommen jetzt aus EnOceanESP3.h.

// Sonderwerte des KO IsTeachChannel (DPT 5.010), neben 1..30 = aktuell aktiver Lern-Kanal.
// Explizit als uint8_t: ein nackter int-Literal waere sonst beim Aufruf von GroupObject::value() zwischen
// KNXValue(int32_t) und KNXValue(uint32_t) mehrdeutig (conversion from 'int' to 'const KNXValue' is ambiguous).
#define ENO_TEACHCHANNEL_CLOSED ((uint8_t)255)            // Lernmodus geschlossen / kein Kanal aktiv
#define ENO_TEACHCHANNEL_ERROR_NO_RESPONSE ((uint8_t)254) // Transceiver hat nicht (rechtzeitig) geantwortet
#define ENO_TEACHCHANNEL_ERROR_REJECTED ((uint8_t)253)    // Transceiver hat den Befehl abgelehnt (Return-Code != RET_OK)

#define RPS_BUTTON_CHANNEL_AI 0
#define RPS_BUTTON_CHANNEL_AO 1
#define RPS_BUTTON_CHANNEL_BI 2
#define RPS_BUTTON_CHANNEL_BO 3

#define RPS_BUTTON_2NDACT_NO 0
#define RPS_BUTTON_2NDACT_VALID 1

#define VLD_CMD_ID_01 0x01
#define VLD_CMD_ID_02 0x02
#define VLD_CMD_ID_03 0x03
#define VLD_CMD_ID_04 0x04
#define VLD_CMD_ID_05 0x05
#define VLD_CMD_ID_06 0x06

#define RPS_Func_10 0xA

//Rocker States
#define ButtonStateO 0
#define ButtonStateI 1

#define AI_pressed 0x10
#define AO_pressed 0x30
#define BI_pressed 0x50
#define BO_pressed 0x70
#define CI_pressed 0x15
#define CO_pressed 0x37
#define Contact_pressed 0xF0

#define AI_release 0x00
#define AO_release 0x20
#define BI_release 0x40
#define BO_release 0x60
#define CI_release 0x05
#define CO_release 0x07
#define Contact_release 0xE0

#define ROCKER_INACTIVE 0x00
#define ROCKER_AI 0x01
#define ROCKER_AO 0x02
#define ROCKER_BI 0x03
#define ROCKER_BO 0x04
#define ROCKER_CI 0x05
#define ROCKER_CO 0x06

class EnoceanModule : public OpenKNX::Module
{
private:
    uint32_t _timer1 = 0;
    uint32_t _timer2 = 0;
    uint8_t _currentChannel = 0;
    EnOceanESP3 _esp3;
    // Secure-Teach-In-Komfortschicht ueber _esp3 (CO_RD_SECUREDEVICES-Diff, Kanal-Zuordnung); aktuell noch ohne
    // eigenes KO/ETS-Parameter angebunden, siehe EnOceanTeachIn.h.
    EnOceanTeachIn _teachIn;
    EnoceanChannel *_channels[ENO_ChannelCount];
    OpenKNX::Flash::Driver *_dummyStorage = nullptr;

    void begin();
    void setupCustomFlash();
    void setupChannels();
    void initSerial(Stream &serial);
    void task();
    bool checkBaseID();
    void startDisableAllChannels();
    uint8_t teachChannelErrorCode() const;
    // An EnOceanTeachIn::onLearnModeChanged() registriert (siehe Konstruktor), damit auch ein ueber
    // EnOceanTeachIn::teachInOnChannel() (Secure-Teach-in) gesteuerter Lernmodus auf KoENO_IsTeachChannel
    // sichtbar wird, genau wie der Weg ueber processInputKo()/startDisableAllChannels(). Reiner Trampolin-Aufruf
    // auf reportTeachInLearnModeChanged(), da der als EnOceanLearnModeCallback registrierte Funktionszeiger
    // statisch sein muss (logDebugP/logInfoP/KoENO_* brauchen dagegen ein 'this', siehe dort).
    static void onTeachInLearnModeChanged(uint8_t channel, EnOceanLearnModeEvent event);
    void reportTeachInLearnModeChanged(uint8_t channel, EnOceanLearnModeEvent event);
    void startReportTeachChannel();
    void getEnOceanMSG(uint8_t u8RetVal, PACKET_SERIAL_TYPE_ *f_Pkt_st);
    bool extractSenderId(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t out8SenderId[4]);
    void checkAndReportUnknownId(PACKET_SERIAL_TYPE_ *f_Pkt_st);
    void handleKnxEvent(uint8_t _channelIndex, int koIndex, GroupObject &ko);
    void send_4BS_Msg(uint8_t *fui8_BaseID_p, uint8_t Index, uint8_t *inputs, uint8_t baseID_CH);
    void send_RPS_Taster(uint8_t *fui8_BaseID_p, boolean state, boolean pressed, uint8_t baseID_CH);
    void setStatusActors(uint8_t *mySenderId, uint8_t idExtra, bool state);
    void getStatusActors(uint8_t *mySenderId, uint8_t idExtra);
    void setActorsMeasurment(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs);
    void getActorsMeasurmentValue(uint8_t *mySenderId, uint8_t idExtra, uint8_t *inputs, bool unit);
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
    void registerUsbExchangeCallbacks();
#endif
#endif

    bool isInited = false;

    // Mit 0 vorbelegt, damit ein fehlgeschlagenes initiales readBaseId() in begin() keine
    // undefinierten Werte hinterlaesst (z.B. fuer nachfolgende send_RPS_Taster()-Aufrufe).
    uint8_t lui8_BaseID_p[BASEID_BYTES] = {0};

    uint32_t _lastPollingTime = 0;

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