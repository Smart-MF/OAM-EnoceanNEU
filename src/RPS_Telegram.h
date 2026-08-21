#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

/*
uint8_t getRockerTyp(uint8_t type)
{
  switch (type)
  {
  case AI_pressed:
    return ROCKER_AI;
    break;
  case AI_release:
    return ROCKER_AI;
    break;
  case AO_pressed:
    return ROCKER_AO;
    break;
  case AO_release:
    return ROCKER_AO;
    break;
  case BI_pressed:
    return ROCKER_BI;
    break;
  case BI_release:
    return ROCKER_BI;
    break;
  case BO_pressed:
    return ROCKER_BO;
    break;
  case BO_release:
    return ROCKER_BO;
    break;
  case CI_pressed:
    return ROCKER_CI;
    break;
  case CI_release:
    return ROCKER_CI;
    break;
  case CO_pressed:
    return ROCKER_CO;
    break;
  case CO_release:
    return ROCKER_CO;
    break;
  default:
    return ROCKER_INACTIVE;
    break;
  }
}*/

// Schreibt einen Bool-Wert (DPT 1.001) auf das übergebene Gruppenobjekt.
inline void shortSend_DPT1(bool value, GroupObject &ko) {
  logDebug("RPS", "DPT1: %u", value);
  ko.value(value, Dpt(1, 1));
}

// Startet einen Dimmvorgang (DPT 3) in die angegebene Richtung auf dem übergebenen Gruppenobjekt.
inline void SendDPT3_007(bool dir, GroupObject &ko) {
  uint8_t dpt3value = 0;
  // UP = 0
  // down = 1
  logDebug("RPS", "DIM: %u", dir);
  dpt3value = 1;
  ko.valueNoSend(dpt3value, Dpt(3, 7, 1));
  if (dir) // Increase
    dpt3value = 8;
  else // decrease
    dpt3value = 0;
  ko.value(dpt3value, Dpt(3, 7, 0));
}

// Liest die per ETS parametrierte Szenennummer aus und schreibt sie auf das übergebene Gruppenobjekt.
inline void shortSend_Szene(uint8_t scene, uint16_t firstParameter, GroupObject &ko) {
  uint8_t szeneNr;

  logDebug("RPS", "Szene: %u", knx.paramByte(firstParameter + scene));
  szeneNr = (knx.paramByte(firstParameter + scene)) - 1;
  ko.value(szeneNr, Dpt(17, 1));
}

// Beendet einen laufenden Dimmvorgang (DPT 3 Stop) auf dem übergebenen Gruppenobjekt.
inline void stopDim(GroupObject &ko) {
  uint8_t dpt3value = 0;

  logDebug("RPS", "Stop DIM");
  dpt3value = 0;
  ko.valueNoSend(dpt3value, Dpt(3, 7, 1));
  dpt3value = 0;
  ko.value(dpt3value, Dpt(3, 7, 0));
}

// Wird beim Loslassen einer lang gedrückten Wippe aufgerufen und stoppt je nach konfigurierter Funktion den laufenden
// Dimmvorgang.
inline void longStop(uint8_t rockerNr, uint16_t firstParameter, uint16_t firstComObj, uint8_t _channelIndex) {
  switch (rockerNr) {
  case AO_pressed:
    logDebug("RPS", "A0"); // AO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA))
    switch (ParamENO_CHRockerFunktionA) {
    case 3: // Long: Licht Heller
      stopDim(KoENO_GO_BASE__2);
      break;
    case 4: // Long: Licht Dunkler
      stopDim(KoENO_GO_BASE__2);
      break;
    }
    break;

  case AI_pressed:
    logDebug("RPS", "AI"); // AI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA))
    switch (ParamENO_CHRockerFunktionA) {
    case 3: // Long: Licht Dunkler
      stopDim(KoENO_GO_BASE__2);
      break;
    case 4: // Long: Licht Heller
      stopDim(KoENO_GO_BASE__2);
      break;
    }
    break;

  case BO_pressed:
    logDebug("RPS", "B0"); // BO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionB))
    switch (ParamENO_CHRockerFunktionB) {
    case 3: // Long: Licht Heller
      stopDim(KoENO_GO_BASE__4);
      break;
    case 4: // Long: Licht Dunkler
      stopDim(KoENO_GO_BASE__4);
      break;
    }
    break;

  case BI_pressed:
    logDebug("RPS", "BI"); // BI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionB))
    switch (ParamENO_CHRockerFunktionB) {
    case 3: // Long: Licht Dunkler
      stopDim(KoENO_GO_BASE__4);
      break;
    case 4: // Long: Licht Heller
      stopDim(KoENO_GO_BASE__4);
      break;
    }
    break;

  case CO_pressed:
    logDebug("RPS", "C0"); // CO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerProfil))
    switch (ParamENO_CHRockerProfil) {
    case Wippen1: // Rocker Profil 1 Wippe
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 3: // Long: Licht Heller
        stopDim(KoENO_GO_BASE__2);
        break;
      case 4: // Long: Licht Dunkler
        stopDim(KoENO_GO_BASE__2);
        break;
      }
      break;

    default: // all other Rocker Profils
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionC))
      switch (ParamENO_CHRockerFunktionC) {
      case 3: // Long: Licht Heller
        stopDim(KoENO_GO_BASE__6);
        break;
      case 4: // Long: Licht Dunkler
        stopDim(KoENO_GO_BASE__6);
        break;
      }
      break;
    }

  case CI_pressed:
    logDebug("RPS", "CI"); // CI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerProfil))
    switch (ParamENO_CHRockerProfil) {
    case Wippen1: // Rocker Profil 1 Wippe
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 3: // Long: Licht Heller
        stopDim(KoENO_GO_BASE__2);
        break;
      case 4: // Long: Licht Dunkler
        stopDim(KoENO_GO_BASE__2);
        break;
      }
      break;
    default: // all other Rocker Profils
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionC))
      switch (ParamENO_CHRockerFunktionC) {
      case 3: // Long: Licht Dunkler
        stopDim(KoENO_GO_BASE__6);
        break;
      case 4: // Long: Licht Heller
        stopDim(KoENO_GO_BASE__6);
        break;
      }
      break;
    }
    break;
  default:
    break;
  }
}

// Wertet einen langen Tastendruck aus und löst je nach konfigurierter Funktion Schalten, Dimmen oder Szenenaufruf aus;
// liefert true, wenn ein Dimmvorgang gestartet wurde.
inline bool longPress(uint8_t rockerNr, uint16_t firstParameter, uint16_t firstComObj, uint8_t _channelIndex) {
  // uint8_t szeneNr;
  switch (rockerNr) {
  case AO_pressed:
    logDebug("RPS", "A0"); // AO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA))
    switch (ParamENO_CHRockerFunktionA) {
    case 1: // Schalten EIN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 2: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 3: // Long: Licht Heller
      SendDPT3_007(true, KoENO_GO_BASE__2);
      return true;
      break;
    case 4: // Long: Licht Dunkler
      SendDPT3_007(false, KoENO_GO_BASE__2);
      return true;
      break;
    case 5: // Long: open = 0
      shortSend_DPT1(false, KoENO_GO_BASE__2);
      break;
    case 6: // Long: close = 1
      shortSend_DPT1(true, KoENO_GO_BASE__2);
      break;
    case 7: // Szene a/b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__1);
      break;
    }
    break;

  case AI_pressed:
    logDebug("RPS", "AI"); // AI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA))
    switch (ParamENO_CHRockerFunktionA) {
    case 1: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 2: // Schalten AN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 3: // Long: Licht Dunkler
      SendDPT3_007(false, KoENO_GO_BASE__2);
      return true;
      break;
    case 4: // Long: Licht Heller
      SendDPT3_007(true, KoENO_GO_BASE__2);
      return true;
      break;
    case 5: // Long: Close = 1
      shortSend_DPT1(true, KoENO_GO_BASE__2);
      break;
    case 6: // Long: Open = 0
      shortSend_DPT1(false, KoENO_GO_BASE__2);
      break;
    case 7: // Szene b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__1);
      break;
    }
    break;

  case BO_pressed:
    logDebug("RPS", "B0"); // BO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionB))
    switch (ParamENO_CHRockerFunktionB) {
    case 1: // Schalten EIN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 2: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 3: // Long: Licht Heller
      SendDPT3_007(true, KoENO_GO_BASE__4);
      return true;
      break;
    case 4: // Long: Licht Dunkler
      SendDPT3_007(false, KoENO_GO_BASE__4);
      return true;
      break;
    case 5: // Long: open = 0
      shortSend_DPT1(false, KoENO_GO_BASE__4);
      break;
    case 6: // Long: close = 1
      shortSend_DPT1(true, KoENO_GO_BASE__4);
      break;
    case 7: // Szene a/b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__3);
      break;
    }
    break;

  case BI_pressed:
    logDebug("RPS", "BI"); // BI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionB))
    switch (ParamENO_CHRockerFunktionB) {
    case 1: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 2: // Schalten AN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 3: // Long: Licht Dunkler
      SendDPT3_007(false, KoENO_GO_BASE__4);
      return true;
      break;
    case 4: // Long: Licht Heller
      SendDPT3_007(true, KoENO_GO_BASE__4);
      return true;
      break;
    case 5: // Long: Close = 1
      shortSend_DPT1(true, KoENO_GO_BASE__4);
      break;
    case 6: // Long: Open = 0
      shortSend_DPT1(false, KoENO_GO_BASE__4);
      break;
    case 7: // Szene b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__3);
      break;
    }
    break;

  case CO_pressed:
    logDebug("RPS", "C0"); // CO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerProfil))
    switch (ParamENO_CHRockerProfil) {
    case Wippen1:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 1: // Schalten EIN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 2: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 3: // Long: Licht Heller
        SendDPT3_007(true, KoENO_GO_BASE__2);
        return true;
        break;
      case 4: // Long: Licht Dunkler
        SendDPT3_007(false, KoENO_GO_BASE__2);
        return true;
        break;
      case 5: // Long: open = 0
        shortSend_DPT1(false, KoENO_GO_BASE__2);
        break;
      case 6: // Long: close = 1
        shortSend_DPT1(true, KoENO_GO_BASE__2);
        break;
      case 7: // Szene a/b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__1);
        break;
      }
      break;
    default:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionC))
      switch (ParamENO_CHRockerFunktionC) {
      case 1: // Schalten EIN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 2: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 3: // Long: Licht Heller
        SendDPT3_007(true, KoENO_GO_BASE__6);
        return true;
        break;
      case 4: // Long: Licht Dunkler
        SendDPT3_007(false, KoENO_GO_BASE__6);
        return true;
        break;
      case 5: // Long: open = 0
        shortSend_DPT1(false, KoENO_GO_BASE__6);
        break;
      case 6: // Long: close = 1
        shortSend_DPT1(true, KoENO_GO_BASE__6);
        break;
      case 7: // Szene a/b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__5);
        break;
      }
      break;
    }
    break;

  case CI_pressed:
    logDebug("RPS", "CI"); // CI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerProfil))
    switch (ParamENO_CHRockerProfil) {
    case Wippen1:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 1: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 2: // Schalten AN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 3: // Long: Licht Dunkler
        SendDPT3_007(false, KoENO_GO_BASE__2);
        return true;
        break;
      case 4: // Long: Licht Heller
        SendDPT3_007(true, KoENO_GO_BASE__2);
        return true;
        break;
      case 5: // Long: Close = 1
        shortSend_DPT1(true, KoENO_GO_BASE__2);
        break;
      case 6: // Long: Open = 0
        shortSend_DPT1(false, KoENO_GO_BASE__2);
        break;
      case 7: // Szene b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__2);
        break;
      }
      break;

    default:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionC))
      switch (ParamENO_CHRockerFunktionC) {
      case 1: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 2: // Schalten AN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 3: // Long: Licht Dunkler
        SendDPT3_007(false, KoENO_GO_BASE__6);
        return true;
        break;
      case 4: // Long: Licht Heller
        SendDPT3_007(true, KoENO_GO_BASE__6);
        return true;
        break;
      case 5: // Long: Close = 1
        shortSend_DPT1(true, KoENO_GO_BASE__6);
        break;
      case 6: // Long: Open = 0
        shortSend_DPT1(false, KoENO_GO_BASE__6);
        break;
      case 7: // Szene b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__5);
        break;
      }
      break;
    }

    break;
  default:
    break;
  }
  return false;
}

// Wertet einen kurzen Tastendruck aus und löst je nach konfigurierter Funktion Schalten, Dimmschritt oder Szenenaufruf
// aus.
inline void shortPress(uint8_t rockerNr, uint16_t firstParameter, uint16_t firstComObj, uint8_t _channelIndex) {
  // uint8_t szeneNr;
  switch (rockerNr) {
  case AO_pressed:
    logDebug("RPS", "A0"); // AO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA))
    switch (ParamENO_CHRockerFunktionA) {
    case 1: // Schalten EIN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 2: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 3: // short: Licht AN
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 4: // short: Licht AUS
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 5: // short: step up = 0
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 6: // short: step down = 1
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 7: // Szene a/b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__1);
      break;
    }
    break;

  case AI_pressed:
    logDebug("RPS", "AI"); // AI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA))
    switch (ParamENO_CHRockerFunktionA) {
    case 1: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 2: // Schalten AN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 3: // short: Licht AUS
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 4: // short: Licht EIN
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 5: // short: step down = 1
      shortSend_DPT1(true, KoENO_GO_BASE__1);
      break;
    case 6: // short: step up = 0
      shortSend_DPT1(false, KoENO_GO_BASE__1);
      break;
    case 7: // Szene b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__1);
      break;
    }
    break;

  case BO_pressed:
    logDebug("RPS", "B0"); // BO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionB))
    switch (ParamENO_CHRockerFunktionB) {
    case 1: // Schalten EIN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 2: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 3: // short: Licht AN
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 4: // short: Licht AUS
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 5: // short: step up = 0
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 6: // short: step down = 1
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 7: // Szene a/b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__3);
      break;
    }
    break;

  case BI_pressed:
    logDebug("RPS", "BI"); // BI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionB))
    switch (ParamENO_CHRockerFunktionB) {
    case 1: // Schalten AUS (Oben/unten Wippe)
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 2: // Schalten AN (Oben/unten Wippe)
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 3: // short: Licht AUS
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 4: // short: Licht EIN
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 5: // short: step down = 1
      shortSend_DPT1(true, KoENO_GO_BASE__3);
      break;
    case 6: // short: step up = 0
      shortSend_DPT1(false, KoENO_GO_BASE__3);
      break;
    case 7: // Szene b (Oben/unten Wippe)
      shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__3);
      break;
    }
    break;

  case CO_pressed:
    logDebug("RPS", "C0, RockerProfil: %u", ParamENO_CHRockerProfil); // CO = normal oben
    // switch (knx.paramByte(firstParameter + ENO_CHRockerProfil))
    switch (ParamENO_CHRockerProfil) {
    case Wippen1:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 1: // Schalten EIN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 2: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 3: // short: Licht AN
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 4: // short: Licht AUS
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 5: // short: step up = 0
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 6: // short: step down = 1
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 7: // Szene a/b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__1);
        break;
      }
      break;

    default:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionC))
      switch (ParamENO_CHRockerFunktionC) {
      case 1: // Schalten EIN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 2: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 3: // short: Licht AN
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 4: // short: Licht AUS
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 5: // short: step up = 0
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 6: // short: step down = 1
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 7: // Szene a/b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneA, firstParameter, KoENO_GO_BASE__5);
        break;
      }
      break;
    }
    break;

  case CI_pressed:
    logDebug("RPS", "CI"); // CI = normal unten
    // switch (knx.paramByte(firstParameter + ENO_CHRockerProfil))
    switch (ParamENO_CHRockerProfil) {
    case Wippen1:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 1: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 2: // Schalten AN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 3: // short: Licht AUS
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 4: // short: Licht EIN
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 5: // short: step down = 1
        shortSend_DPT1(true, KoENO_GO_BASE__1);
        break;
      case 6: // short: step up = 0
        shortSend_DPT1(false, KoENO_GO_BASE__1);
        break;
      case 7: // Szene b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__1);
        break;
      }
      break;

    default:
      // switch (knx.paramByte(firstParameter + ENO_CHRockerFunktionA)) // FunctionA because of Wippen1
      switch (ParamENO_CHRockerFunktionA) // FunctionA because of Wippen1
      {
      case 1: // Schalten AUS (Oben/unten Wippe)
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 2: // Schalten AN (Oben/unten Wippe)
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 3: // short: Licht AUS
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 4: // short: Licht EIN
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 5: // short: step down = 1
        shortSend_DPT1(true, KoENO_GO_BASE__5);
        break;
      case 6: // short: step up = 0
        shortSend_DPT1(false, KoENO_GO_BASE__5);
        break;
      case 7: // Szene b (Oben/unten Wippe)
        shortSend_Szene(ENO_CHRockerASzeneB, firstParameter, KoENO_GO_BASE__5);
        break;
      }
      break;
    }
    break;
  default:
    break;
  }
}

// Verarbeitet das Loslassen einer Taste (kurz oder lang) abhängig von der konfigurierten Funktion: Schalten, Dimmen,
// Jalousie oder Szenenaufruf.
inline void release_Button(bool stateIO, uint16_t firstParameter, uint16_t firstComObj, uint8_t RockerFunktion,
                           uint8_t RockerSzene, bool islong, uint8_t _channelIndex) {
  uint8_t szeneNr;
  uint8_t dpt3value = 0;

  switch (knx.paramByte(firstParameter + RockerFunktion)) {
  case 1:        // Schalten EIN/AUS (Oben/unten Wippe)
    if (stateIO) // Button I
    {
      logDebug("RPS", "Schalten AUS");
      KoENO_GO_BASE__2.value(false, Dpt(1, 1));
    } else // Button O
    {
      logDebug("RPS", "Schalten EIN");
      KoENO_GO_BASE__2.value(true, Dpt(1, 1));
    }
    break;

  case 2:        // Schalten AUS/EIN (Oben/unten Wippe)
    if (stateIO) // Button I
    {
      logDebug("RPS", "Schalten EIN");
      KoENO_GO_BASE__2.value(true, Dpt(1, 1));
    } else // Button O
    {
      logDebug("RPS", "Schalten AUS");
      KoENO_GO_BASE__2.value(false, Dpt(1, 1));
    }
    break;

  case 3: // short: Licht AN/AUS   long: Dimmen Heller/Dunkler (Oben/unten Wippe)
    if (islong) {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Dimmen heller");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 7, 1));
        dpt3value = 8;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 7, 0));
      } else {
        logDebug("RPS", "Dimmen Dunkler");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 7, 1));
        dpt3value = 0;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 7, 0));
      }
    } else {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Licht  AUS");
        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      } else {
        logDebug("RPS", "Licht  AN");
        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      }
    }
    break;

  case 4: // short: Licht AN/AUS   long: Dimmen Heller/Dunkler (Oben/unten Wippe)
    if (islong) {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Dimmen dunkler");
        dpt3value = 1;
        //+2
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 7, 1));
        dpt3value = 0;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 7, 0));
      } else {
        logDebug("RPS", "Dimmen heller");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 7, 1));
        dpt3value = 8;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 7, 0));
      }
    } else {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Licht AN");
        //+1
        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      } else {
        logDebug("RPS", "Licht  AUS");
        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      }
    }
    break;

  case 5: // short: step AB/AUF   long: Jalousie AUF/AB (Oben/unten Wippe)
    if (islong) {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Jalousie AB");
        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      } else {
        logDebug("RPS", "Jalousie AUF");
        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      }
    } else {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Jalousie Step AB");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 8, 1));
        dpt3value = 8;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 8, 0));
      } else {
        logDebug("RPS", "Jalousie Step AUF");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 8, 1));
        dpt3value = 0;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 8, 0));
      }
    }
    break;

  case 6: // Jalousie AB/AUF (Oben/unten Wippe)
    if (islong) {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Jalousie AB");
        KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      } else {
        logDebug("RPS", "Jalousie AUF");
        KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      }
    } else {
      if (stateIO) // Button I
      {
        logDebug("RPS", "Jalousie Step AUF");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 8, 1));
        dpt3value = 0;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 8, 0));
      } else {
        logDebug("RPS", "Jalousie Step AB");
        dpt3value = 1;
        KoENO_GO_BASE__3.valueNoSend(dpt3value, Dpt(3, 8, 1));
        dpt3value = 8;
        KoENO_GO_BASE__3.value(dpt3value, Dpt(3, 8, 0));
      }
    }
    break;

  case 7: // Szene a/b (Oben/unten Wippe)
    logDebug("RPS", "Szene %s: %u", stateIO ? "b" : "a", knx.paramByte(firstParameter + RockerSzene));
    szeneNr = (knx.paramByte(firstParameter + RockerSzene)) - 1;
    KoENO_GO_BASE__2.value(szeneNr, Dpt(17, 1));

    break;

  default:
    break;
  }
}

// Behandelt ein RPS-Wippentelegramm für einen einzelnen Kontakt (gedrückt/losgelassen) und schaltet das zugehörige
// Gruppenobjekt.
inline void handle_RPS_Rocker(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t profil, uint8_t firstComObj,
                              uint8_t firstParameter, uint8_t index, uint8_t _channelIndex) {

  switch (f_Pkt_st->u8DataBuffer[1]) {
  case Contact_pressed:
    shortSend_DPT1(true, KoENO_GO_BASE__2);
    break; // ENDE pressed

  case Contact_release:
    shortSend_DPT1(false, KoENO_GO_BASE__2);
    break; // ENDE released
  }
}

// Verarbeitet ein F6-02-03-Telegramm (zwei Wippen an einem Kontakt) und schaltet die beiden zugehörigen Gruppenobjekte
// je nach Tastencode.
inline void handle_F6_02_03(uint8_t value, uint8_t _channelIndex) {
  logDebug("RPS", "Profil: F6-02-03");
  switch (value) {
  case 0x30: // A0 Set
    KoENO_GO_BASE__1.value(true, Dpt(1, 1));
    logDebug("RPS", "A0 (0x30)");
    break;
  case 0x10: // A0 release
    KoENO_GO_BASE__1.value(false, Dpt(1, 1));
    logDebug("RPS", "A1 (0x10)");
    break;
  case 0x70: // B0 Set
    KoENO_GO_BASE__2.value(true, Dpt(1, 1));
    logDebug("RPS", "B0 (0x70)");
    break;
  case 0x50: // B0 relesae
    KoENO_GO_BASE__2.value(false, Dpt(1, 1));
    logDebug("RPS", "B1 (0x50)");
    break;
  default:
    break;
  }
}

// Verarbeitet die F6-05-xx-Telegramme (Rauch-/Wassermelder bzw. Energiestatus) und schreibt den passenden
// Alarm-/Statuswert.
inline void handle_F6_05_0x(uint8_t value, uint8_t _channelIndex) {
  switch (value) {
  case 0x00: // SMOKE/WATER Alarm OFF 3 2
    KoENO_GO_BASE__3.value(true, DPT_Bool);
    break;
  case 0x10: // SMOKE/WATER Alarm ON
    KoENO_GO_BASE__3.value(false, DPT_Bool);
    break;
  case 0x11: // Water detected
    KoENO_GO_BASE__3.value(true, DPT_Bool);
    break;
  case 0x30: // Energy LOW
    KoENO_GO_BASE__4.value(true, DPT_Bool);
    break;
  default:
    break;
  }
}

// Zentrale Dispatch-Funktion für empfangene RPS-Telegramme dieses Kanals: wertet das konfigurierte EEP-Profil aus und
// schaltet die zugehörigen Gruppenobjekte bzw. ruft die passende Behandlungsroutine auf.
inline void handle_RPS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  RPS_F6_10_00_TYPE *lRpsTlg_p;
  RPS_F6_10_01_TYPE *lRpsTlg2_p;

  bool bvalue;

#ifndef EnOceanTEST
  // switch (knx.paramWord(firstParameter + ENO_CHProfilSelectionRPS))
  switch (ParamENO_CHProfilSelectionRPS)
#else
  switch (profil)
#endif
  {
  case F6_02_01:
    logDebug("RPS", "Profil: F6-02-01");
    break;
  case F6_02_02:
    logDebug("RPS", "Profil: F6-02-02");
  case F6_02_03:
    handle_F6_02_03(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
  case F6_03_01:
    logDebug("RPS", "Profil: F6-03-01");
    break;
  case F6_03_02:
    logDebug("RPS", "Profil: F6-03-02");
    break;
  //**************************************************************
  // ----------------- Profil: F6-05-00   ------------------------
  //**************************************************************
  case F6_05_00:
    logDebug("RPS", "Profil: F6-05-00");
    handle_F6_05_0x(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
  //**************************************************************
  // ----------------- Profil: F6-05-01 --------------------------
  //**************************************************************
  case F6_05_01:
    logDebug("RPS", "Profil: F6-05-01");
    handle_F6_05_0x(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
    //**************************************************************
  // ----------------- Profil: F6-05-02   ------------------------
  //**************************************************************
  case F6_05_02:
    logDebug("RPS", "Profil: F6-05-02");
    handle_F6_05_0x(f_Pkt_st->u8DataBuffer[1], _channelIndex);
    break;
  //**************************************************************
  // ----------------- Profil: F6-10-00 --------------------------
  //**************************************************************
  case F6_10_00:

    logDebug("RPS", "Profil: F6-10-00");

    lRpsTlg_p = (RPS_F6_10_00_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

    logDebug("RPS", "STATE: %u, NA: %u", lRpsTlg_p->u8RpsTelData.STATE, lRpsTlg_p->u8RpsTelData.NA);

    switch (lRpsTlg_p->u8RpsTelData.STATE) {
    case 0b1111: // Hebel unten
      KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
        bvalue = true;
      else
        bvalue = false;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel unten");
      break;
    case 0b1101: // Hebel Oben
      KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      KoENO_GO_BASE__3.value(true, Dpt(1, 1));
      if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel oben");
      break;
    case 0b1100: // Hebel Mitte
      KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel mitte");
      break;
    case 0b1110: // Hebel Mitte
      KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      if (((ParamENO_CHWindowcloseValue) >> ENO_CHWindowcloseValueShift) & 1)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel mitte");
      break;
    default:
      logDebug("RPS", "RPS ERROR");
      break;
    }
    break;
  //**************************************************************
  // ----------------- Profil: F6-10-01 --------------------------
  //**************************************************************
  case F6_10_01:
    logDebug("RPS", "Profil: F6-10-01");

    lRpsTlg2_p = (RPS_F6_10_01_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

    switch (lRpsTlg2_p->u8RpsTelData.STATE) {
    case 0b11: // Hebel unten
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      KoENO_GO_BASE__4.value(false, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = true;
      else
        bvalue = false;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel unten");
      break;
    case 0b01: // Hebel Oben
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      KoENO_GO_BASE__4.value(true, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel oben");
      break;
    case 0b00: // Hebel Mitte
      KoENO_GO_BASE__3.value(true, Dpt(1, 1));
      KoENO_GO_BASE__4.value(false, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel mitte");
      break;
    case 0b10: // Hebel Mitte
      KoENO_GO_BASE__3.value(true, Dpt(1, 1));
      KoENO_GO_BASE__4.value(false, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel mitte");
      break;
    default:
      // Statement(s)
      break;
    }
    break;
  default:
    logDebug("RPS", "Profil: ERROR");
    break;
  }
}

inline void handle_F6_10_00(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex) {
  RPS_F6_10_00_TYPE *lRpsTlg_p;
  bool bvalue;

  logDebug("RPS", "Profil: F6-10-00");

  lRpsTlg_p = (RPS_F6_10_00_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

  logDebug("RPS", "STATE: %u, NA: %u", lRpsTlg_p->u8RpsTelData.STATE, lRpsTlg_p->u8RpsTelData.NA);

  switch (lRpsTlg_p->u8RpsTelData.STATE) {
  case 0xF0: // geschlossen
    if (ParamENO_CHWindowcloseValue)
      bvalue = true;
    else
      bvalue = false;
    KoENO_GO_BASE__2.value(bvalue, Dpt(1, 1));
    logDebug("RPS", "geschlossen");
    break;
  case 0xE0: // 0ffen
    if (ParamENO_CHWindowcloseValue)
      bvalue = true;
    else
      bvalue = false;
    KoENO_GO_BASE__2.value(bvalue, Dpt(1, 1));
    logDebug("RPS", "offen");
  default:
    logDebug("RPS", "RPS ERROR");
    break;
  }




    logDebug("RPS", "Profil: F6-10-00");

    lRpsTlg_p = (RPS_F6_10_00_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

    logDebug("RPS", "STATE: %u, NA: %u", lRpsTlg_p->u8RpsTelData.STATE, lRpsTlg_p->u8RpsTelData.NA);
    
    
    switch (lRpsTlg_p->u8RpsTelData.STATE) {
    case 0b1111: // Hebel unten
      KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = true;
      else
        bvalue = false;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel unten");
      break;
    case 0b1101: // Hebel Oben
      KoENO_GO_BASE__2.value(false, Dpt(1, 1));
      KoENO_GO_BASE__3.value(true, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel oben");
      break;
    case 0b1100: // Hebel Mitte
      KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel mitte");
      break;
    case 0b1110: // Hebel Mitte
      KoENO_GO_BASE__2.value(true, Dpt(1, 1));
      KoENO_GO_BASE__3.value(false, Dpt(1, 1));
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
      KoENO_GO_BASE__4.value(bvalue, Dpt(1, 1));
      logDebug("RPS", "Hebel mitte");
      break;
    default:
      logDebug("RPS", "RPS ERROR");
      break;
    }
}
