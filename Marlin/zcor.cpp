#include "zcor.h"
#include "temperature.h"

Zcor zcor; // singleton

// public:

void Zcor::init(){
    SERIAL_ECHOLNPGM("Z correction init");
    spi.init();
    OUT_WRITE(SS_PIN, HIGH);
    OUT_WRITE(ZCOR_SS_PIN, HIGH);
};
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
void Zcor::test() {
    SERIAL_ECHOLNPGM("Z correction test");
    WRITE(ZCOR_SS_PIN, LOW); // enable spi
    safe_delay(10000);
    WRITE(ZCOR_SS_PIN, HIGH); // disable spi
}

// private:

const uint8_t Zcor::configured_microsteps[] = MICROSTEP_MODES;

int Zcor::currentCorrectionSteps[Zlr] = { 0 };

int Zcor::correctionStepsZr(const float height) {
    return 0;
}

SPI<MISO_PIN, MOSI_PIN, SCK_PIN> Zcor::spi;