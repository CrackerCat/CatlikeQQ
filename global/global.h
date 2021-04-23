#ifndef GLOBAL_H
#define GLOBAL_H

#include "runtime.h"
#include "usersettings.h"
#include "signaltransfer.h"

extern QString APPLICATION_NAME;
extern QString VERSION_CODE;

extern UserSettings* us;
extern Runtime* rt;
extern SignalTransfer* sig;

#endif // GLOBAL_H