#ifndef ZCOR_H
#define ZCOR_H

class Zcor {
    public:
        static void reset();
        static void correct(const float height);

    private:
        static float correctionNeededZr(const float height);
};

extern Zcor zcor;

#endif // ZCOR_H