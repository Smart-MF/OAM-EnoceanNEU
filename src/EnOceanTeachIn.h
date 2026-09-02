#pragma once
/*
 * EnOceanTeachIn.h
 *
 * Anwendungsnahe Schicht ueber EnOceanESP3: oeffnet/schliesst den
 * TCM515-Lernmodus (CO_WR_LEARNMODE), erlaubt die Vorgabe eines konkreten
 * Kanals fuer das naechste Teach-In und liest die Liste bekannter sicherer
 * Geraete aus (CO_RD_SECUREDEVICES).
 *
 * WICHTIG zum Kanal-Parameter (Spec Tab. 52, "Channel"):
 * Das TCM515 verarbeitet ankommende Secure-Teach-In-Telegramme selbststaendig
 * (Schluessel, Rolling Code, CMAC) - dafuer ist der Channel-Wert irrelevant.
 * Er dient nur dazu, dem Modul waehrend des offenen Lernfensters mitzuteilen,
 * "das jetzt gelernte Geraet gehoert zu Kanal N". Diese Zuordnung wird vom
 * TCM515 aber NICHT dauerhaft am Geraet gespeichert (CO_RD_SECUREDEVICES
 * liefert nur SLF+ID zurueck, keinen Kanal) - deshalb fuehrt TeachIn die
 * Zuordnung selbst mit (siehe mappingCount()/mapping()). Fuer einen Reset-
 * festen Speicher muesst ihr das zusaetzlich selbst persistieren (Flash/EEPROM).
 *
 * Hinweis: teachInOnChannel() ist eine blockierende Komfort-Funktion (haelt
 * fuer windowMs den Aufrufer an); fuer die Verwendung aus EnoceanModule::loop()
 * heraus (wo das den KNX-Bus fuer die Dauer des Lernfensters anhalten wuerde)
 * eignet sich stattdessen EnOceanESP3::activateLearnMode() direkt (siehe
 * EnoceanModule::processInputKo()/startDisableAllChannels()).
 */

#include "EnOceanESP3.h"

struct EnOceanSecureDevice {
    uint8_t slf = 0;  // Security Level Format
    uint32_t id = 0;  // Geraete-ID (Base-ID des Sensors)
};

struct EnOceanDeviceChannelMap {
    uint32_t id = 0;
    uint8_t channel = 0;
};

// Wird bei einem CO_EVENT_SECUREDEVICES-Event aufgerufen (Fehler waehrend des Lernfensters, z.B. kein
// Speicherplatz mehr frei). cause: Spec Tab. 18.
typedef void (*EnOceanSecureDeviceEventCallback)(uint32_t deviceId, uint8_t cause);

class EnOceanTeachIn {
public:
    explicit EnOceanTeachIn(EnOceanESP3 &esp3) : _esp3(esp3) {}

    void onSecureDeviceEvent(EnOceanSecureDeviceEventCallback cb) { _eventCb = cb; }

    // CO_RD_SECUREDEVICES: alle beim TCM515 bekannten sicheren Geraete.
    int readSecureDevices(EnOceanSecureDevice *out, int maxDevices);

    // Oeffnet den Lernmodus fuer GENAU EINEN Kanal, wartet windowMs auf ein Teach-In-Telegramm (Lerntaste am
    // Zielgeraet druecken!), schliesst danach automatisch wieder. Erkennt per Diff der Secure-Device-Liste
    // vor/nach dem Fenster die neu gelernte Geraete-ID und merkt sich Geraet<->Kanal intern
    // (mappingCount()/mapping()). Gibt true zurueck, wenn genau ein neues Geraet gefunden wurde.
    // ACHTUNG: blockiert den Aufrufer fuer bis zu windowMs (siehe Hinweis oben).
    bool teachInOnChannel(uint8_t channel, uint32_t windowMs, uint32_t *newDeviceIdOut = nullptr);

    int mappingCount() const { return _mapCount; }
    const EnOceanDeviceChannelMap &mapping(int index) const { return _map[index]; }

private:
    static const int MAX_MAPPED_DEVICES = 32;

    EnOceanESP3 &_esp3;
    EnOceanSecureDeviceEventCallback _eventCb = nullptr;
    EnOceanDeviceChannelMap _map[MAX_MAPPED_DEVICES];
    int _mapCount = 0;

    void rememberChannel(uint32_t id, uint8_t channel);
    void pumpEventsDuringWindow(uint32_t windowMs);
};
