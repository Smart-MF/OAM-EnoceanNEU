# Enocean Gateway



## ELTAKO 

### Taster

### Tür und Fensterkontakte

**FFG7B - Fenstergriffsensor:**
EEP: A5-14-09 (Default) Zyklisch: 15min
EEP: F6-10-00 (Option1) Zyklisch: NEIN 
EEP: F6-10-00 (Option2) Zyklisch: 15min 

In der linken oberen Ecke des GehäusesS
befi ndet sich eine Öffnung, über die ein
Mikro taster zugänglich ist. Dieser dient zur
Navigation durch die im Folgenden
beschriebene Menüstruktur.
Hauptmenü:
Aufrufen: Taste ca. 1 Sekunde gedrückt halten, dann loslassen LED-Bestätigung: 1× blinken

*Untermenü 1: Verschlüsselung*
Aufrufen: Taste 1× kurz drücken Ɓ 1 × blinken
Optionen:
1 × drücken Ɓ Verschlüsselung aus (voreingestellt) Ɓ LED: 1 × kurz + 1 × langS
2 × drücken Ɓ Verschlüsselung ein Ɓ
LED: 2 × kurz + 1× lang

*Untermenü 2: Protokoll (EEP)*
Aufrufen: Taste 2 × kurz drücken Ɓ
2 × blinken
Optionen:
1 × drücken Ɓ 'EEP A5-14-09' (voreingestellt)
Ɓ LED: 1 × kurz + 1× lang
2 × drücken Ɓ 'EEP F6-10-00' (sendet nicht
zyklisch). Ɓ LED: 2 × kurz + 1× lang
3 × drücken Ɓ 'EEP F6-10-00' (sendet zyklisch
alle 15min). Ɓ LED: 3 × kurz + 1× lang.

Telegramme nach EEP: A5-14-09
Lerntelegramm: 0x50480D80
Datentelegramme:
Data_byte0 = 0x08 = Fenster geschlossen
0x0E = Fenster offen
0x0A = Fenster gekippt
Data_byte3 = Batteriespannung:
0..250, 0..5 V

Telegramme nach EEP: F6-10-00
'Lerntelegramm': 0xF0
ORG = 0x05
Data_byte3 = 0xF0 = Fenster geschlossen
             0xE0 = Fenster offen
             0xD0 = Fenster gekippt

**FFKB - Tür/Fensterkontakt**
Sensor-Telegramme (EEP: D5-00-01)
Lerntelegramm: 0x00000000
ORG = 0x06
Data_byte3 = Kontakt geschlossen = 0x09
Kontakt offen = 0x08
Spannungs-Telegramm:
(Auswertung z.B. mit der GFVS)
ORG = 0x07
Data_byte0 = 0x08
Data_byte1 = 0x00
Batteriespannung:
Data_byte2 = 0x00..0xFF entspricht 0..5 V
Energiespeicher:
Data_byte3 = 0x00.. 0xFF entspricht 0..5 V

*Hauptmenü*
Aufrufen: Taste ca. 1 Sekunde gedrückt halten,
dann loslassen
LED-Bestätigung: 1× blinken
*Untermenü 1: Verschlüsselung*
Aufrufen: Taste 1× kurz drücken Ɓ 1× blinken
Optionen:
1× drücken Ɓ Verschlüsselung aus (voreingestellt) Ɓ LED: 1× kurz + 1× lang
2× drücken Ɓ Verschlüsselung ein
Ɓ LED: 2× kurz + 1× lang
*Untermenü 2: Protokoll (EEP)*
Aufrufen: Taste 2× kurz drücken Ɓ 2× blinken
Optionen:
1× drücken Ɓ EEP D5-00-01 + 4BS Telegramm Spannungswerte (voreingestellt)
Ɓ LED: 1× kurz + 1× lang
2× drücken Ɓ EEP D5-00-01 Ɓ LED: 2× kurz
+ 1× lang 

**FTK - Tür/Fensterkontakt**
gleich wie FFKB


**FTKB - Tür/Fensterkontakt** 
gleich wie FFKB

**FTKE - Fenster-Türkontakt**
Funk-Telegramme
nach EEP F6-10-00
ORG = 0x05
Data_byte3 = 
0xF0 = Fenster geschlossen
0xE0 = Fenster offen

**FFTE - Fenster-Tastkontakt**
gleich wie FTKE

