#include "zcor.h"
#include "Marlin.h"

Zcor zcor; // singleton

// public:

void Zcor::reset(){
    SERIAL_ECHOLNPGM("Z correction reset");
};
void Zcor::correct(){
    SERIAL_ECHOLNPGM("Z correction correct");
};

