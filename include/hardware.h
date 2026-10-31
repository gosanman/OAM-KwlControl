#pragma once

// Auswahl des Board-Headers. OGM-HardwareConfig wird NICHT eingebunden: die Platine ist einmalig, ihr Header definiert alles selbst — auch die
// OpenKNX-Systempins (KNX-UART, PROG, SAVE, LEDs).

#if defined(DEVICE_KNXFANDRV_REV01)
    #include "KnxFanDrv_Rev01.h"
#else
    #error "Kein Board gewaehlt: -D DEVICE_KNXFANDRV_REV01 in platformio.custom.ini setzen"
#endif
