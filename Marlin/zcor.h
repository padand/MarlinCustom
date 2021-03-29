#ifndef ZCOR_H
#define ZCOR_H

#include "MarlinConfig.h"
#include "MarlinSPI.h"
#include <axis_value_parser.h>
#include <zcor_protocol.h>

class Zcor {
    public:
        static void init();
        static void reset();
        static void correct(const float height);
        static void test(uint8_t axis);
        // reads the axis position; returns true if successfull
        static bool readPosition(const uint8_t axis);

    private:
        static int currentCorrectionSteps[ZZZ];
        static const uint8_t configured_microsteps[];
        static int correctionStepsZr(const float height);
        static SPI<MISO_PIN, MOSI_PIN, SCK_PIN> spi;
        static AxisValueParser avp;
};

extern Zcor zcor;

#endif // ZCOR_H