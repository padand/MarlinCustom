#ifndef ZCOR_H
#define ZCOR_H

#include "MarlinConfig.h"
#include "MarlinSPI.h"
#include <axis_value_parser.h>
#include <zcor_protocol.h>

class CorrectionRequired {
    public:
        char getSteps(AxisZEnum axis);
        void setSteps(AxisZEnum axis, char steps);
    private:
        char steps[ZZZ];
};

class Correction {
    public:
        static void setRequired(float height, const CorrectionRequired cr);
        static CorrectionRequired getRequired(float height);
        static void sdWriteRequired();
        static void sdReadRequired();
    private:
        static const int requiredLen = round(float(ZCOR_Z_HEIGHT)/float(ZCOR_LAYER_HEIGHT)) + 1;
        static CorrectionRequired required[requiredLen];
        static char sdFileName[];
};

class Zcor {
    public:
        static void init();
        // return true if probe was successful
        static bool probe(const float height);
        // return true if tune was successful
        static bool tune(const float height, const uint8_t cycles, bool *tuned);
        static void store();
        static void restore();
        static void correct(const float height);
        static void reset();
        // reads the axis position; returns true if successfull
        static bool readAxisPosition(const AxisZEnum axis, float *position);
        static bool verifyAllAxesAt0();

    private:
        static void settle(const unsigned int d = ZCOR_SETTLE_DELAY);
        static char currentCorrectionSteps[ZZZ];
        static const uint8_t configured_microsteps[];
        static SPI<MISO_PIN, MOSI_PIN, SCK_PIN> spi;
        static AxisValueParser avp;
        static Correction correction;
};

extern Zcor zcor;

#endif // ZCOR_H