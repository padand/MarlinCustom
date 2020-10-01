#ifndef ZCOR_H
#define ZCOR_H

#include "MarlinConfig.h"

class Zcor {
    public:
        static void reset();
        static void correct(const float height);

    private:
        static int currentCorrectionSteps[Zlr];
        static const uint8_t configured_microsteps[];
        static int correctionStepsZr(const float height);
};

extern Zcor zcor;

#endif // ZCOR_H