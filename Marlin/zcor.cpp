#include "zcor.h"
#include "Marlin.h"

Zcor zcor; // singleton

// public:

void Zcor::reset(){
    SERIAL_ECHOLNPGM("Z correction reset");
};
void Zcor::correct(const float height){
    SERIAL_ECHOLNPAIR("Z correction correct ", height);
    const int c = correctionStepsZr(height);
    SERIAL_ECHOLNPAIR("Z correction correct Zr ", c);
};

// private:

int Zcor::correctionStepsZr(const float height) {
    const double estimatedError = ZCOR_ZR_A * sin(ZCOR_ZR_B * double(height) + ZCOR_ZR_C) + ZCOR_ZR_D;
    SERIAL_ECHOLN(String(estimatedError, 4));
    return int(LROUND(estimatedError/Z_STEP_CORRECTION_UNIT));
}