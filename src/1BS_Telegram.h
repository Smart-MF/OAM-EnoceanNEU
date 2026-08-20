#pragma once

#include "EnoceanModule.h"
#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "knxprod.h"

/*******************************************************************************************
 *  1BS   D5-00-01
 *******************************************************************************************/

inline void handle_1BS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex)
{
  bool bvalue;

  ONEBS_TELEGRAM_TYPE *l1bsTlg_p;

#ifdef KDEBUG
  SERIAL_PORT.print("Profil: 1BS - ");
  SERIAL_PORT.println(firstParameter + ENO_CHProfilSelection4BS);
#endif

  switch (ParamENO_CHProfilSelection1BS)
  {
  case D5_00_01:

    // Läd den Wert in das Profil-Strukt
    l1bsTlg_p = (ONEBS_TELEGRAM_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

    // ETS Parameter to define state for open / Close
    // D5-00-01 Contact: 0 = open / 1 = close
    if (l1bsTlg_p->u81bsTelData.State == 1) // CLOSE
    {
      if (ParamENO_CHWindowcloseValue)
        bvalue = true;
      else
        bvalue = false;
    }
    else
    { // OPEN
      if (ParamENO_CHWindowcloseValue)
        bvalue = false;
      else
        bvalue = true;
    }
    KoENO_GO_BASE__3.valueNoSend(bvalue, DPT_Bool);
    KoENO_GO_BASE__3.objectWritten();

#ifdef KDEBUG
    SERIAL_PORT.println("D5-00-01");
    SERIAL_PORT.print(F("detected: 1BS State: "));
    SERIAL_PORT.println(l1bsTlg_p->u81bsTelData.State);
    SERIAL_PORT.print(F("Output KNX: "));
    SERIAL_PORT.println(bvalue);
#endif
    break;

  default:
    break;
  }
}