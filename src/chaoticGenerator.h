#ifndef CHAOTIC_GEN_H
#define CHAOTIC_GEN_H
#include <random>
#include <math.h>
#include <climits>
#include <stdexcept>
#include "customStruct.h"

class PRBG{
    std::minstd_rand0* generator;
public:
    uint_fast32_t getNextByte() {
        return (*generator)();
    }
    PRBG(std::minstd_rand0* devicePtr, std::int32_t key) {
        generator = devicePtr;
    }
    PRBG() {
        generator = nullptr;
    }
};

// Vanilla PWLCM
class chaotic_PWLM{
    double chaoticOrbit;
    double pValue;

    double pwlcm_F(double x, double p) {
        if ((x >= 0) && (x < p)) {
            return (x/p);
        } else if ((x >= p) && (x <= 0.5)) {
            return ((x-p) / (0.5 - p));
        } else if ((x >= 0.5) && (x <= 1)) {
            return pwlcm_F(1-x, p);
        }
    }

public:
    double getNextOrbit() {
        chaoticOrbit = pwlcm_F(chaoticOrbit, pValue);
        return chaoticOrbit;
    }
    chaotic_PWLM(double initCond, double pVal) {
        chaoticOrbit = initCond;
        pValue = pVal;
        return;
    }
    chaotic_PWLM() {
        chaoticOrbit = 0;
        pValue = 0;
        return;
    }
};

// PWLCM with perturbation, precision is 64-bits
class chaoticGen{
    chaotic_PWLM* pwlcm_Funct;
    std::minstd_rand0* generator;
    unsigned int perturbInterval;
    unsigned int curDelta;    // Tracking current interval for perturbing
    uint_fast32_t perturbMask;
    unsigned int l_perturbLen;

public:
    double getNext() {
        double nextNum = pwlcm_Funct->getNextOrbit();
        if (curDelta == perturbInterval) {
            // Perturb the lowest l-bits
            uint64_t nextNum_int64 = *reinterpret_cast<uint64_t*>(&nextNum);
            uint_fast32_t perturbSignal = (*generator)();
            nextNum_int64 = (nextNum_int64 & ~perturbMask) | (perturbSignal & perturbMask);
            nextNum = *reinterpret_cast<double*>(&nextNum_int64);
            curDelta = -1;
        }
        curDelta++;
        return nextNum;
    }

    chaoticGen(std::pair<double, double> msgKey, unsigned int perturbDelta) {
        generator = new std::minstd_rand0(msgKey.first);
        // Fetching: initial conditions x_0 & p-value of F(x) from msgKey
        pwlcm_Funct = new chaotic_PWLM(msgKey.second, msgKey.first);
        perturbInterval = perturbDelta;
        curDelta = 0;
        // Perturbing length l is fixed at 10 bits (dont know how to calculate the lyapunov_exp xddd)
        l_perturbLen = 10;
        perturbMask = ((uint_fast32_t)1 << l_perturbLen) - 1;
        return;
    }
    chaoticGen() {
        generator = nullptr;
        pwlcm_Funct = nullptr;
        perturbInterval = 0;
        curDelta = 0;
        l_perturbLen = 0;
        return;
    }

};

class CPRBG{
    chaoticGen* generator1;
    chaoticGen* generator2;
    unsigned int m_discardIter; // value m in CCS-PRBG
    bool g_funct(double x_1, double x_2) {
        if (x_1 > x_2) {
            return true;
        } else if (x_1 < x_2) {
            return false;
        } else {
            throw std::runtime_error("Err: x_1 == x_2 in g_funct()\n");
        }
    }
    // Returns char w/ MSB = bit obtained, the rest are zeros
    unsigned char getNextBit() {
        // Discard m-bits of {k(i)}
        for (int i = 0; i < m_discardIter-1; i++) {
            g_funct(generator1->getNext(), generator2->getNext());
        }
        return (g_funct(generator1->getNext(), generator2->getNext())) ? 0x80: 0x00;
    }

public:
    unsigned char getNextByte() {
        unsigned char retVal = 0;
        unsigned char tracker = 0x80;
        for (int i = 0; i < CHAR_BIT; i++) {
            if (this->getNextBit()) {
                retVal |= tracker;
            }
            tracker = tracker >> 1;
        }
        return retVal;
    }
    CPRBG(std::pair<double, double> msgKey_1, std::pair<double, double> msgKey_2, std::pair<unsigned int, unsigned int> perturbDelta) {
        generator1 = new chaoticGen(msgKey_1, perturbDelta.first);
        generator2 = new chaoticGen(msgKey_2, perturbDelta.second);
        m_discardIter = (perturbDelta.first > perturbDelta.second) ? perturbDelta.first+1: perturbDelta.second+1;
        return;
    }
    CPRBG() {
        generator1 = generator2 = nullptr;
        m_discardIter = 0;
        return;
    }
};

#endif