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
        static void setLayerHeight(const float h);
        static float getLayerHeight();
        static void setRequired(float height, const CorrectionRequired cr);
        static CorrectionRequired getRequired(float height);
        static void printRequired();
    private:
        static const int requiredMax = int(ZCOR_MAX_HEIGHT/ZCOR_MAX_LAYER_HEIGHT);
        static CorrectionRequired required[requiredMax];
        static float layerHeight;
};

class Zcor {
    public:
        static void init();
        static void probeLayerHeight(const float layerHeight);
        static void probe(const float height);
        static void correct(const float height);
        // reads the axis position; returns true if successfull
        static bool readAxisPosition(const AxisZEnum axis, float *position);
        static bool verifyAllAxesAt0();

    private:
        static int currentCorrectionSteps[ZZZ];
        static const uint8_t configured_microsteps[];
        static int correctionStepsZr(const float height);
        static SPI<MISO_PIN, MOSI_PIN, SCK_PIN> spi;
        static AxisValueParser avp;
        static Correction correction;
};

extern Zcor zcor;

#endif // ZCOR_H