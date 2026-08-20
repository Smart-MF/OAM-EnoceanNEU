#pragma once

#include "EnoceanProfils.h"
#include "EnoceanStructs.h"
#include "EnoceanModule.h"
#include "knxprod.h"

/*******************************************************************************************
 *  1BS   D5-00-01 
 *******************************************************************************************/

inline void handle_1BS(PACKET_SERIAL_TYPE_ *f_Pkt_st, uint8_t _channelIndex)
{
    //bool bvalue;

    ONEBS_TELEGRAM_TYPE *l1bsTlg_p;

#ifdef KDEBUG
    SERIAL_PORT.print("Profil: 1BS - ");
    SERIAL_PORT.println(firstParameter + ENO_CHProfilSelection4BS);
#endif

    switch (ParamENO_CHProfilSelection1BS )
    {
    case D5_00_01:
#ifdef KDEBUG
        SERIAL_PORT.println("D5-00-01");
#endif
        // Läd den Wert in das Profil-Strukt
        l1bsTlg_p = (ONEBS_TELEGRAM_TYPE *)&(f_Pkt_st->u8DataBuffer[1]);

 //       // ETS Parameter to define state for open / Close
 //       if (l1bsTlg_p->u81bsTelData.State == 1) // CLOSE
 //       // D5-00-01 Contact: 0 = open / 1 = close
 //       {
 //           if (((knx.paramByte(firstParameter + ENO_CHWindowcloseValue)) >> ENO_CHWindowcloseValueShift) & 1)
 //               bvalue = true;
 //           else
 //               bvalue = false;
 //       }
 //       else
 //       { // OPEN
 //           if (((knx.paramByte(firstParameter + ENO_CHWindowcloseValue)) >> ENO_CHWindowcloseValueShift) & 1)
 //               bvalue = false;
 //           else
 //               bvalue = true;
 //       }
        KoENO_GO_BASE__3.valueNoSend(l1bsTlg_p->u81bsTelData.State , DPT_Bool);
        KoENO_GO_BASE__3.objectWritten();

#ifdef KDEBUG
        SERIAL_PORT.print(F("detected: 1BS State: "));
        SERIAL_PORT.println(l1bsTlg_p->u81bsTelData.State);
#endif
        break;

   

    default:
        break;
    }
}