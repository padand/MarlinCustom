#include "zcor.h"
#include "temperature.h"

Zcor zcor; // singleton

#ifndef ZCOR_SS_PIN
    #define ZCOR_SS_PIN 0
#endif

#ifndef ZCOR_SPI_TIMEOUT
    #define ZCOR_SPI_TIMEOUT 1500
#endif

#ifdef ZCOR_ENABLE_DEBUG
  #define DEBUG_MSG(msg) SERIAL_ECHOLNPGM(msg)
  #define DEBUG_PAIR(msg, data) SERIAL_ECHOLNPAIR(msg, data)
#else
  #define DEBUG_MSG(msg)
  #define DEBUG_PAIR(msg, data)
#endif

// public:

void Zcor::init(){
    SERIAL_ECHOLNPGM("Z correction init");
    spi.init();
    spi.setRate(SPI_EIGHTH_SPEED);
    spi.setBitOrder(SPI_MSBFIRST);
    spi.setDataMode(SPI_MODE0);
    OUT_WRITE(SS_PIN, HIGH);
    OUT_WRITE(ZCOR_SS_PIN, HIGH);
};
void Zcor::probe(const float height) {
    float value;
    LOOP_Z(axis) {
        while(!readAxisPosition((AxisZEnum)axis, &value));
        SERIAL_ECHOLNPAIR("probed axis: ", axis);
        SERIAL_ECHOLNPAIR("got value: ", value);
        SERIAL_ECHOLNPAIR("diff: ", height - value);
    }
}
void Zcor::correct(const float height){
    SERIAL_ECHOLNPAIR("Z correction correct at height ", height);
    const int csZr = height == 0 ? 0 : correctionStepsZr(height) * configured_microsteps[Z_AXIS];
    thermalManager.babystep_Zi(Z1_AXIS, csZr - currentCorrectionSteps[Z1_AXIS]);
    currentCorrectionSteps[Z1_AXIS] = csZr;
};
bool Zcor::readAxisPosition(const AxisZEnum axis, float *position) {
    WRITE(SS_PIN, HIGH);
    WRITE(ZCOR_SS_PIN, LOW); // enable spi
    delay(100);
    // Request position pos continuously
    SERIAL_ECHOLNPGM("Request position");
    spi.transfer(REQUEST_POSITION_READ((int)axis));
    if(!spi.waitResponse(REQUEST_POSITION_STATUS,RESPONSE_POSITION_STATUS_OK((int)axis), ZCOR_SPI_TIMEOUT)) {
        SERIAL_ECHOLNPGM("Position request timeout");
        return false;
    }
    SERIAL_ECHOLNPGM("Position aquired");
    uint8_t res;
    avp.init();
    unsigned long timeout = ZCOR_SPI_TIMEOUT + millis(); 
    while(!avp.verify()) {
        if(millis() > timeout) {
            SERIAL_ECHOLNPGM("Position verify timeout");
            DEBUG_PAIR("last spi res: ", res);
            return false;
        }
        res = spi.transfer(REQUEST_POSITION_DIGIT);
        if(RESPONSE_IS_POSITION_DIGIT(res)) {
            avp.add(RESPONSE_POSITION_DIGIT(res));
        }
        // delay(50);
    }
    WRITE(ZCOR_SS_PIN, HIGH); // disable spi
    *position = avp.pos();
    return true;
}
bool Zcor::verifyAllAxesAt0() {
    float value;
    LOOP_Z(axis) {
        while(!readAxisPosition((AxisZEnum)axis, &value));
        // verify correct initialization
        if (value != 0) {
            return false;            
        }
    }
    return true;
};

// private:

const uint8_t Zcor::configured_microsteps[] = MICROSTEP_MODES;

int Zcor::currentCorrectionSteps[ZZZ] = { 0 };

int Zcor::correctionStepsZr(const float height) {
    return 0;
}

SPI<MISO_PIN, MOSI_PIN, SCK_PIN> Zcor::spi;

AxisValueParser Zcor::avp;