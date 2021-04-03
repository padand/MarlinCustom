#include "zcor.h"
#include "temperature.h"
#include "cardreader.h"

Zcor zcor; // singleton

#ifndef ZCOR_SS_PIN
    #define ZCOR_SS_PIN 0
#endif

#ifndef ZCOR_SPI_TIMEOUT
    #define ZCOR_SPI_TIMEOUT 1500
#endif

#ifndef ZCOR_SPI_SPEED
    #define ZCOR_SPI_SPEED SPI_SPEED_EIGHTH
#endif

#ifdef ZCOR_ENABLE_DEBUG
  #define DEBUG_MSG(msg) SERIAL_ECHOLNPGM(msg)
  #define DEBUG_PAIR(msg, data) SERIAL_ECHOLNPAIR(msg, data)
#else
  #define DEBUG_MSG(msg)
  #define DEBUG_PAIR(msg, data)
#endif

//=============================================================== CLASS CorrectionRequired

// PUBLIC

char CorrectionRequired::getSteps(AxisZEnum axis) {
    return steps[axis];
}

void CorrectionRequired::setSteps(AxisZEnum axis, char steps) {
    this->steps[axis] = steps;
}

//=============================================================== CLASS Correction

// PUBLIC

void Correction::setRequired(float height, const CorrectionRequired cr) {
    int index = round(height / float(ZCOR_LAYER_HEIGHT));
    if (index < requiredLen) required[index] = cr;
};

CorrectionRequired Correction::getRequired(float height) {
    int index = round(height / float(ZCOR_LAYER_HEIGHT));
    if (index >= requiredLen) index = 0;
    return required[index];
};

void Correction::sdWriteRequired() {
    #if !ENABLED(SDSUPPORT)
        SERIAL_ECHOLNPGM("Z correction store requires SD support");
    #else

    if(!card.cardOK) card.initsd();
    if(!card.cardOK) {
        SERIAL_ECHOLNPGM("There was a problem initializing SD card");
        return;
    }
    if (card.isFileOpen()) card.closefile();
    card.openFile(sdFileName, false);
    if (!card.isFileOpen()) {
        SERIAL_ECHOLNPGM("There was a problem opening the file");
        return;
    };
        
    for(unsigned int i=0; i<requiredLen; i++) {
        LOOP_Z(axis) {
            if(!card.write_byte(required[i].getSteps((AxisZEnum)axis))) {
                SERIAL_ECHOLNPGM("There was a problem writing to the file");
                i=requiredLen;
                break;
            };
        }
    }

    card.closefile();
    SERIAL_ECHOLNPGM("DONE SD write");

    #endif
}

void Correction::sdReadRequired() {
    #if !ENABLED(SDSUPPORT)
        SERIAL_ECHOLNPGM("Z correction store requires SD support");
    #else

    if(!card.cardOK) card.initsd();
    if(!card.cardOK) {
        SERIAL_ECHOLNPGM("There was a problem initializing SD card");
        return;
    }
    if (card.isFileOpen()) card.closefile();
    card.openFile(sdFileName, true);
    if (!card.isFileOpen()) {
        SERIAL_ECHOLNPGM("There was a problem opening the file");
        return;
    };

    uint8_t b;
        
    for(unsigned int i=0; i<requiredLen; i++) {
        LOOP_Z(axis) {
            if(!card.read_byte(&b)) {
                SERIAL_ECHOLNPGM("There was a problem reading the file");
                i=requiredLen;
                break;
            };
            required[i].setSteps((AxisZEnum)axis, b);
        }
    }

    card.closefile();
    SERIAL_ECHOLNPGM("DONE SD read");

    #endif
}

// PRIVATE

CorrectionRequired Correction::required[requiredLen];
char Correction::sdFileName[] = { ZCOR_FILENAME };

//=============================================================== CLASS Zcor

// PUBLIC

void Zcor::init(){
    SERIAL_ECHOLNPGM("Z correction init");
    spi.init();
    spi.setRate(ZCOR_SPI_SPEED);
    spi.setBitOrder(SPI_MSBFIRST);
    spi.setDataMode(SPI_MODE0);
    OUT_WRITE(ZCOR_SS_PIN, HIGH);
};
bool Zcor::probe(const float height) {
    CorrectionRequired cr;
    float value;

    // COARSE CORRECTION
    // read the coarse correction values
    settle();
    LOOP_Z(axis) {
        if(!readAxisPosition((AxisZEnum)axis, &value)){
            SERIAL_ECHOLNPGM("Halt probing due to position read error");
            return false;
        }
        cr.setSteps((AxisZEnum)axis, round((height - value) / float(ZCOR_UNIT)));
    }
    // apply the coarse correction steps
    LOOP_Z(axis){
        thermalManager.babystep_Zi((AxisZEnum)axis, cr.getSteps((AxisZEnum)axis) * configured_microsteps[Z_AXIS]);
    }
    while(thermalManager.babystep_Zi_in_progress()) idle();

    // FINE CORRECTION
    settle();
    bool isFine = false;
    char step;
    while(!isFine) {
        isFine = true;
        LOOP_Z(axis) {
            if(!readAxisPosition((AxisZEnum)axis, &value)){
                SERIAL_ECHOLNPGM("Halt probing due to position read error");
                return false;
            }
            if(fabs(height-value) < float(ZCOR_UNIT)/2.0f){
                step = 0;
            } else if(height > value) {
                step = 1;
            } else {
                step = -1;
            }
            if (step != 0) {
                isFine = false;
                cr.setSteps((AxisZEnum)axis, cr.getSteps((AxisZEnum)axis) + step);
                thermalManager.babystep_Zi((AxisZEnum)axis, step * configured_microsteps[Z_AXIS]);
                while(thermalManager.babystep_Zi_in_progress()) idle();
                settle();
            }
        }
    }

    // revert the babystepping to original position
    LOOP_Z(axis) {
        thermalManager.babystep_Zi((AxisZEnum)axis, (-1) * cr.getSteps((AxisZEnum)axis) * configured_microsteps[Z_AXIS]);
    }
    while(thermalManager.babystep_Zi_in_progress()) idle();

    // record the results
    correction.setRequired(height, cr);

    return true;
}
void Zcor::store(){
    correction.sdWriteRequired();
};
void Zcor::restore(){
    correction.sdReadRequired();
};
void Zcor::correct(const float height){
    SERIAL_ECHOLNPAIR("Z correction correct at height ", height);
    LOOP_Z(axis) {
        thermalManager.babystep_Zi((AxisZEnum)axis, (correction.getRequired(height).getSteps((AxisZEnum)axis) - currentCorrectionSteps[axis]) * configured_microsteps[Z_AXIS]);
        currentCorrectionSteps[axis] = correction.getRequired(height).getSteps((AxisZEnum)axis);
    }
};
void Zcor::reset() {
    SERIAL_ECHOLNPGM("Z correction reset");
    LOOP_Z(axis) {
        currentCorrectionSteps[axis] = 0;
    }
}
bool Zcor::readAxisPosition(const AxisZEnum axis, float *position) {
    #if ENABLED(SDSUPPORT)
        WRITE(SDSS, HIGH); // disable sdcard spi
    #endif
    delay(100);
    WRITE(ZCOR_SS_PIN, LOW); // enable spi
    delay(100);
    // Request position pos continuously
    spi.transfer(REQUEST_POSITION_READ((int)axis));
    if(!spi.waitResponse(REQUEST_POSITION_STATUS,RESPONSE_POSITION_STATUS_OK((int)axis), ZCOR_SPI_TIMEOUT)) {
        SERIAL_ECHOLNPGM("Position request timeout");
        WRITE(ZCOR_SS_PIN, HIGH); // disable spi
        return false;
    }
    uint8_t res;
    avp.init();
    unsigned long timeout = ZCOR_SPI_TIMEOUT + millis(); 
    while(!avp.verify()) {
        if(millis() > timeout) {
            SERIAL_ECHOLNPGM("Position verify timeout");
            DEBUG_PAIR("last spi res: ", res);
            WRITE(ZCOR_SS_PIN, HIGH); // disable spi
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
        if(!readAxisPosition((AxisZEnum)axis, &value)){
            return false;
        };
        // verify correct initialization
        if (value != 0) {
            return false;            
        }
    }
    return true;
};

// PRIVATE

void Zcor::settle() {
    unsigned long time = millis() + ZCOR_SETTLE_DELAY;
    while (PENDING(millis(), time)) idle();
}

const uint8_t Zcor::configured_microsteps[] = MICROSTEP_MODES;

char Zcor::currentCorrectionSteps[ZZZ] = { 0 };

SPI<MISO_PIN, MOSI_PIN, SCK_PIN> Zcor::spi;

AxisValueParser Zcor::avp;

Correction Zcor::correction;