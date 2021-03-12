//
// Created by hjkim17 on 21. 2. 3..
//

#ifndef SHD_DDES_MODE_H_
#define SHD_DDES_MODE_H_

typedef enum _DdesMode DdesMode;
enum _DdesMode {
    DDES_UNSET,
    DDES_FULL,
    DDES_SLAVE,
};

const char* ddesmode_toStr(DdesMode mode);
DdesMode ddesmode_fromStr(const char* ddesStr);

#endif /* SHD_DDES_MODE_H_ */
