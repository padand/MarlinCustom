#ifndef ZCOR_H
#define ZCOR_H

#include "MarlinConfig.h"
#include "MarlinSPI.h"

class Zcor {
    public:
        static void init();
        static void reset();
        static void correct(const float height);
        static void test();

    private:
        static int currentCorrectionSteps[Zlr];
        static const uint8_t configured_microsteps[];
        static int correctionStepsZr(const float height);
        static SPI<MISO_PIN, MOSI_PIN, SCK_PIN> spi;
};

extern Zcor zcor;

#endif // ZCOR_H