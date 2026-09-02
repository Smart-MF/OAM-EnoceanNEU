#pragma once
/*
 * EnOceanESP3.h
 *
 * Low-Level-Treiber fuer ESP3 (EnOcean Serial Protocol 3) auf Basis eines
 * beliebigen Arduino Stream (z.B. HardwareSerial). Implementiert nach:
 * "EnOcean Serial Protocol 3 (ESP3)" V1.21, Feb 12 2013.
 *
 * Abgedeckte Kapitel der Spec:
 *  - 1.3    Packet structure (Sync, Header, CRC8H, Data, Optional Data, CRC8D)
 *  - 1.6    UART synchronization
 *  - 1.10.9  / 1.10.10 CO_WR_IDBASE / CO_RD_IDBASE
 *  - 1.10.11 / 1.10.12 CO_WR_REPEATER / CO_RD_REPEATER
 *  - 1.10.25 / 1.10.26 CO_WR_LEARNMODE / CO_RD_LEARNMODE
 *  - 2.3    CRC8 calculation (Polynom x^8+x^2+x^1+x^0)
 *  - 2.4    UART Synchronization (Referenz-C-Code fuer Empfangs-Statemachine)
 *
 * Alle mehrbytigen Felder (Data Length, ...) werden gemaess Spec MSB-first
 * (big endian) uebertragen.
 *
 * Die Klasse verwaltet den UART selbst nicht (kein begin()/setRX()/setTX() an
 * der Stream-Schnittstelle) - das bleibt bewusst Sache des Aufrufers
 * (EnoceanModule::setup()), damit der bestehende Setup-Ablauf unveraendert
 * bleibt. EnOceanESP3::begin() erwartet einen bereits initialisierten Stream.
 */

#include "OpenKNX.h"

// ---- ESP3-Statuscodes der Empfangs-Statemachine / des Paketversands ------
#define ENOCEAN_OK 0
#define ENOCEAN_OUT_OF_RANGE 21
#define ENOCEAN_NOT_VALID_CHKSUM 7
#define ENOCEAN_NO_RX_TEL 6
#define ENOCEAN_NEW_RX_BYTE 3

// ---- Packet Type, Spec Kap. 1.6.2 (nur die hier benoetigten Werte) -------
#define u8RADIO_ERP1 0x01
#define u8RESPONSE 0x02
#define u8RORG_COMMON_COMMAND 0x05 // = Packet Type COMMON_COMMAND

// ---- COMMON_COMMAND Codes, Spec Kap. 1.10.2 (nur die hier benoetigten) ---
#define u8CO_WR_IDBASE 0x07    // Kap. 1.10.9
#define u8CO_RD_IDBASE 0x08    // Kap. 1.10.10
#define u8CO_WR_REPEATER 0x09  // Kap. 1.10.11
#define u8CO_RD_REPEATER 0x0A  // Kap. 1.10.12, 10
#define u8CO_WR_LEARNMODE 0x17 // Kap. 1.10.25, 23
#define u8CO_RD_LEARNMODE 0x18 // Kap. 1.10.26, 24
#define u8CO_RD_SECUREDEVICES 0x1B // Kap. 1.10.29, 27

// ---- Packet Type EVENT + der fuer EnOceanTeachIn benoetigte Event-Code,
// Spec Kap. 1.9.2 / 1.9.7 -----------------------------------------------
#define u8EVENT 0x04
#define u8EVT_CO_EVENT_SECUREDEVICES 0x05

#define BASEID_BYTES 4
// Muss mindestens 1 + EnOceanTeachIn::MAX_MAPPED_DEVICES * 5 Bytes fassen, damit eine volle
// CO_RD_SECUREDEVICES-Antwort (Spec Tab. 58) nicht am Puffer scheitert (poll() liefert sonst
// ENOCEAN_OUT_OF_RANGE statt eines - ggf. nur teilweisen - Ergebnisses).
#define DATBUF_SZ 200

#define SER_SYNCH_CODE 0x55
#define SER_HEADER_NR_BYTES 4

// Default-Timeout (ms) fuer die blockierenden Request/Response-Helper.
#define ENOCEAN_ESP3_RESPONSE_TIMEOUT 500

// Detailergebnis des letzten activateLearnMode()/readLearnMode()-Aufrufs (Diagnose, z.B. fuer eine
// Fehlermeldung per KO). Bei Erfolg immer LEARNMODE_RESULT_OK.
enum EnOceanLearnResult : uint8_t
{
    LEARNMODE_RESULT_OK = 0,
    LEARNMODE_RESULT_NO_RESPONSE = 1, // Transceiver hat nicht/nicht rechtzeitig geantwortet
    LEARNMODE_RESULT_REJECTED = 2,    // Transceiver hat geantwortet, aber mit Return-Code != RET_OK
};

//! Zustaende der Empfangs-Statemachine, Spec Kap. 2.4.2.
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

//! Wire-Paketstruktur (ESP3): Data und Optional Data liegen gemeinsam,
//! direkt hintereinander, im Puffer u8DataBuffer (Spec Kap. 1.6.1 / 2.4.1).
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

/*
 * Low-Level ESP3-Treiber: Sync/Header/CRC8H/Data/OptionalData/CRC8D senden
 * und empfangen (Spec Kap. 1.3, 1.6, 2.4) sowie High-Level-Helfer fuer die
 * in diesem Projekt benoetigten COMMON_COMMANDs (Base-ID, Repeater,
 * Lernmodus).
 */
class EnOceanESP3
{
public:
    // serial muss bereits mit der ESP3-Baudrate (57600) initialisiert sein
    // (siehe EnoceanModule::setup()); diese Klasse fasst RX/TX-Pins und
    // Serial.begin() bewusst nicht an.
    void begin(Stream &serial);

    // ---- Low-Level: ein Rohpaket senden/empfangen -------------------------

    // Blockierender Versand (Sync/Header/CRC8H/Data/OptionalData/CRC8D),
    // Spec Kap. 2.4.3. Liefert ENOCEAN_OK oder ENOCEAN_OUT_OF_RANGE.
    uint8_t sendPacket(PACKET_SERIAL_TYPE_ *pPacket);

    // Nicht-blockierend: verarbeitet pro Aufruf alle aktuell im UART-Puffer
    // wartenden Bytes und haelt den Statemachine-Zustand zwischen den
    // Aufrufen. Liefert ENOCEAN_OK, wenn packet() ein vollstaendiges,
    // CRC-korrektes Paket enthaelt; sonst ENOCEAN_NO_RX_TEL / _NEW_RX_BYTE /
    // _NOT_VALID_CHKSUM / _OUT_OF_RANGE.
    uint8_t poll();

    // Zuletzt vollstaendig empfangenes Paket. Gueltig, nachdem poll() bzw.
    // einer der High-Level-Helfer unten ENOCEAN_OK/true geliefert hat.
    PACKET_SERIAL_TYPE_ &packet() { return _rxPacket; }

    // ---- High-Level: COMMON_COMMAND senden und blockierend (Timeout in ms)
    // auf die zugehoerige RESPONSE warten ------------------------------

    // CO_RD_IDBASE (Kap. 1.10.10). Liefert bei Erfolg true und die 4 Base-ID
    // Bytes in fui8_BaseID_p.
    bool readBaseId(uint8_t fui8_BaseID_p[BASEID_BYTES], uint32_t timeoutMs = ENOCEAN_ESP3_RESPONSE_TIMEOUT);

    // CO_WR_IDBASE (Kap. 1.10.9). Liefert den ESP3-Return-Code der Antwort
    // (0x00 = RET_OK) bzw. ENOCEAN_NO_RX_TEL, wenn keine Antwort kam.
    uint8_t setBaseId(const uint8_t fui8_BaseID_p[BASEID_BYTES], uint32_t timeoutMs = ENOCEAN_ESP3_RESPONSE_TIMEOUT);

    // CO_WR_LEARNMODE (Kap. 1.10.25). channel: 1..0xFD absolut, 0xFE = voriger, 0xFF = naechster Kanal (relativ).
    // deviceTimeoutMs: im Telegramm mitgesendetes Timeout, nach dem der Transceiver den Lernmodus selbststaendig
    // wieder beendet (0 = Geraete-Default von 60000 ms). responseTimeoutMs: wie lange hier maximal auf die
    // RESPONSE des Transceivers gewartet wird.
    // Garantiert bei enable=true (fuer absolute Kanaele, 1..0xFD), dass zu jedem Zeitpunkt hoechstens ein
    // Kanal im Lernmodus ist: ein zuvor ueber diese Methode aktivierter, abweichender Kanal wird dafuer
    // immer zuerst deaktiviert.
    bool activateLearnMode(uint8_t channel, bool enable, uint32_t deviceTimeoutMs = 0,
                            uint32_t responseTimeoutMs = 100);

    // CO_RD_LEARNMODE (Kap. 1.10.26).
    bool readLearnMode(uint8_t &enable, uint8_t &channel, uint32_t timeoutMs = ENOCEAN_ESP3_RESPONSE_TIMEOUT);

    // Detailergebnis des letzten activateLearnMode()- oder readLearnMode()-Aufrufs; nur zur Diagnose bei
    // enable/readLearnMode == false gedacht (z.B. um per KO zwischen "keine Antwort vom Transceiver" und
    // "vom Transceiver abgelehnt" zu unterscheiden).
    EnOceanLearnResult lastLearnModeResult() const { return _lastLearnModeResult; }

    // CO_WR_REPEATER (Kap. 1.10.11). enable: OFF=0/ON=1, level: 0 (OFF), 1, 2.
    bool setRepeater(uint8_t enable, uint8_t level, uint32_t timeoutMs = ENOCEAN_ESP3_RESPONSE_TIMEOUT);

    // CO_RD_REPEATER (Kap. 1.10.12).
    bool readRepeater(uint8_t &enable, uint8_t &level, uint32_t timeoutMs = ENOCEAN_ESP3_RESPONSE_TIMEOUT);

    // CO_RD_SECUREDEVICES (Kap. 1.10.29): liefert alle beim Transceiver bekannten sicheren Geraete
    // (SLF + Device-ID) in outSlf[]/outId[] (parallele Arrays, je maxDevices Eintraege). Rueckgabe: Anzahl
    // gelieferter Eintraege, oder -1 bei Fehler/Timeout.
    int readSecureDevices(uint8_t outSlf[], uint32_t outId[], int maxDevices, uint32_t timeoutMs = ENOCEAN_ESP3_RESPONSE_TIMEOUT);

    static uint8_t crc8(uint8_t crc, uint8_t dataByte);

    // Fuer logDebugP/logInfoP/... (siehe OpenKNX::Log::Logger.h); analog zu
    // z.B. OpenKNX::Flash::Driver::logPrefix().
    std::string logPrefix();

private:
    // Sendet ein COMMON_COMMAND-Requestpaket und wartet blockierend (max.
    // timeoutMs) auf die zugehoerige RESPONSE (asynchron eintreffende
    // Fremdpakete - z.B. Funktelegramme - werden dabei verworfen). Bei
    // Erfolg zeigt packet() auf die Antwort.
    bool sendAndWaitResponse(PACKET_SERIAL_TYPE_ *pRequest, uint32_t timeoutMs);

    // Rohes CO_WR_LEARNMODE, ohne die Ein-Kanal-Garantie von activateLearnMode().
    bool sendLearnMode(uint8_t channel, bool enable, uint32_t deviceTimeoutMs, uint32_t responseTimeoutMs);

    Stream *_serial = nullptr;

    // Kanal, fuer den activateLearnMode() zuletzt erfolgreich Lernmodus=an gesendet hat (0 = keiner). Nur
    // fuer absolute Kanaele (1..0xFD) gepflegt; dient dazu, vor dem Aktivieren eines anderen Kanals immer
    // zuerst den bisherigen zu deaktivieren.
    uint8_t _activeLearnChannel = 0;

    // Siehe lastLearnModeResult().
    EnOceanLearnResult _lastLearnModeResult = LEARNMODE_RESULT_OK;

    // Empfangs-Statemachine-Zustand (bleibt zwischen poll()-Aufrufen erhalten)
    STATES_GET_PACKET_ _rxState = GET_SYNC_STATE;
    uint16_t _rxCount = 0;
    uint8_t _rxCrc = 0;
    uint8_t _rxHeader[SER_HEADER_NR_BYTES] = {0};

    // Puffer/Struktur fuer das zuletzt vollstaendig empfangene Paket - wird
    // sowohl fuer asynchron eintreffende Funktelegramme (poll(), aus
    // EnoceanModule::task()) als auch fuer Antworten der High-Level-Helfer
    // oben verwendet (identisch zum bisherigen Verhalten).
    uint8_t _rxBuffer[DATBUF_SZ];
    PACKET_SERIAL_TYPE_ _rxPacket;

    static uint8_t u8CRC8Table[256];
};
