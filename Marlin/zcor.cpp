#include "zcor.h"
#include "temperature.h"

Zcor zcor; // singleton

// public:

void Zcor::reset(){
    SERIAL_ECHOLNPGM("Z correction reset");
    correct(0);
};
void Zcor::correct(const float height){
    SERIAL_ECHOLNPAIR("Z correction correct at height ", height);
    const int csZr = height == 0 ? 0 : correctionStepsZr(height) * configured_microsteps[Z_AXIS];
    thermalManager.babystep_Zlr(Zr_AXIS, csZr - currentCorrectionSteps[Zr_AXIS]);
    currentCorrectionSteps[Zr_AXIS] = csZr;
};

// private:

const uint8_t Zcor::configured_microsteps[] = MICROSTEP_MODES;

int Zcor::currentCorrectionSteps[Zlr] = { 0 };

int Zcor::correctionStepsZr(const float height) {
    const double estimatedError = ZCOR_ZR_A * sin(ZCOR_ZR_B * double(height) + ZCOR_ZR_C) + ZCOR_ZR_D;
    //SERIAL_ECHOLN(String(estimatedError, 4));
    return int(LROUND(estimatedError/Z_STEP_CORRECTION_UNIT));
}