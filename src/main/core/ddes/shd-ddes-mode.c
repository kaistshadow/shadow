//
// Created by hjkim17 on 21. 2. 3..
//

#include "shadow.h"

const char* ddesmode_toStr(DdesMode mode) {
    switch (mode) {
        case DDES_FULL:
            return "full";
        case DDES_SLAVE:
            return "slave";
        case DDES_UNSET:
        default:
            return "unset";
    }
}
DdesMode ddesmode_fromStr(const char* ddesStr) {
    if(ddesStr == NULL) {
        return DDES_UNSET;
    } else if (g_ascii_strcasecmp(ddesStr, "full") == 0) {
        return DDES_FULL;
    } else if (g_ascii_strcasecmp(ddesStr, "slave") == 0) {
        return DDES_SLAVE;
    } else {
        return DDES_UNSET;
    }
}