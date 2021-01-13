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
    CBI(
      #ifdef PRR
        PRR
      #elif defined(PRR0)
        PRR0
      #endif
        , PRSPI);
    SPCR = _BV(MSTR) | _BV(SPE) | _BV(SPR0);
    WRITE(ZCOR_SS_PIN, LOW); // enable spi
    safe_delay(100);
    SERIAL_ECHOLNPGM("Request reset");
    if(!spi.waitResponse(REQUEST_RESET,RESPONSE_RESET,1000)) {
        SERIAL_ECHOLNPGM("Reset timeout");
        return;
    }
    SERIAL_ECHOLNPGM("Reset ok");
    // Request position pos continuously
    uint8_t pos = 1;
    SERIAL_ECHOLNPGM("Request position");
    spi.transfer(REQUEST_POSITION_READ(pos));
    if(!spi.waitResponse(REQUEST_POSITION_STATUS,RESPONSE_POSITION_STATUS_OK(pos),5000)) {
        SERIAL_ECHOLNPGM("Position aquire timeout");
        return;
    }
    SERIAL_ECHOLNPGM("Position aquired");
    uint8_t res;
    avp.init();
    while(!avp.verify()) {
        res = spi.transfer(REQUEST_POSITION_DIGIT);
        //Serial.println(res);
        if(RESPONSE_IS_POSITION_DIGIT(res)) {
            avp.add(RESPONSE_POSITION_DIGIT(res));
        }
        // delay(50);
    }
    float value = avp.pos();
    SERIAL_ECHOLNPAIR("Got value: ", value);
    WRITE(ZCOR_SS_PIN, HIGH); // disable spi
}

// private:

const uint8_t Zcor::configured_microsteps[] = MICROSTEP_MODES;

int Zcor::currentCorrectionSteps[Zlr] = { 0 };

int Zcor::correctionStepsZr(const float height) {
    return 0;
}

SPI<MISO_PIN, MOSI_PIN, SCK_PIN> Zcor::spi;

AxisValueParser Zcor::avp;