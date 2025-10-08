#pragma once
#include "EnoceanChannel.h"
#include "OpenKNX.h"
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
#include "UsbExchangeModule.h"
#endif
#endif

#define ENOCEAN_OK 0
#define ENOCEAN_OUT_OF_RANGE 21
#define ENOCEAN_NOT_VALID_CHKSUM 7
#define ENOCEAN_NO_RX_TEL 6
#define ENOCEAN_NEW_RX_BYTE 3

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
#define u8RORG_COMMON_COMMAND 0x05
#define u8KNX_RORG_Rocker 1
#define u8KNX_RORG_VLD 2
#define u8KNX_RORG_4BS 3

/*
    Packet type
*/

#define u8RADIO_ERP1 0x01
#define u8RESPONSE 0x02

// COMANDS
#define u8CO_WR_IDBASE 0x07
#define u8CO_RD_IDBASE 0x08
#define u8CO_WR_REPEATER 0x09
#define u8CO_RD_REPEATER 0x0A // 10

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

#define BASEID_BYTES 4
#define DATBUF_SZ 100

#define SER_SYNCH_CODE 0x55
#define SER_HEADER_NR_BYTES 4

//! uart_getPacket state machine states.
enum STATES_GET_PACKET_
{
    //! Waiting for the synchronisation byte 0x55
    GET_SYNC_STATE = 0,
    //! Copying the 4 after sync byte: raw data length (2 bytes), optional data length (1), type (1).
    GET_HEADER_STATE,
    //! Checking the header CRC8 checksum. Resynchronisation test is also done here
    CHECK_CRC8H_STATE,
    //! Copying the data and optional data bytes to the paquet buffer
    GET_DATA_STATE,
    //! Checking the info CRC8 checksum.
    CHECK_CRC8D_STATE,
};

//! Packet structure (ESP3)
struct PACKET_SERIAL_TYPE_
{
    // Amount of raw data bytes to be received. The most significant byte is sent/received first
    uint16_t u16DataLength;
    // Amount of optional data bytes to be received
    uint8_t u8OptionLength;
    // Packetype code
    uint8_t u8Type;
    // Data buffer: raw data + optional bytes
    uint8_t *u8DataBuffer;
};

class EnoceanModule : public OpenKNX::Module
{
private:
    uint32_t _timer1 = 0;
    uint32_t _timer2 = 0;
    uint8_t _currentChannel = 0;
    Stream *_serial;
    EnoceanChannel *_channels[ENO_ChannelCount];
    OpenKNX::Flash::Driver *_dummyStorage = nullptr;

    void begin();
    void setupCustomFlash();
    void setupChannels();
    void initSerial(Stream &serial);
    void task();
    void readBaseId(uint8_t *fui8_BaseID_p);
    void getEnOceanMSG(uint8_t u8RetVal, PACKET_SERIAL_TYPE_ *f_Pkt_st);
    uint8_t uart_getPacket(PACKET_SERIAL_TYPE_ *pPacket, uint16_t u16BufferLength);
    uint8_t uart_sendPacket(PACKET_SERIAL_TYPE_ *pPacket);
#ifdef ARDUINO_ARCH_RP2040
#ifndef OPENKNX_USB_EXCHANGE_IGNORE
    void registerUsbExchangeCallbacks();
#endif
#endif

    bool isInited = false;

    uint8_t u8datBuf[DATBUF_SZ];
    uint8_t u8CRC;
    uint8_t u8RxByte;
    uint8_t u8RetVal;

    STATES_GET_PACKET_ u8State;
    PACKET_SERIAL_TYPE_ m_Pkt_st;

    static uint8_t u8CRC8Table[256];

    uint8_t lui8_BaseID_p[BASEID_BYTES];

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