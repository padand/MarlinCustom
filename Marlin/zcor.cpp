#include "zcor.h"
#include "Marlin.h"

Zcor zcor; // singleton

// public:

void Zcor::reset(){
    SERIAL_ECHOLNPGM("Z correction reset");
};
void Zcor::correct(const float height){
    SERIAL_ECHOLNPAIR("Z correction correct ", height);
};

// private:

float Zcor::correctionNeededZr(const float height) {
    return ZCOR_ZR_A * sin(ZCOR_ZR_B * height * ZCOR_ZR_C) + ZCOR_ZR_D;
}