#pragma once

#include "FixedDelayLine.h"

// Special Freeverb comb filter
class LBCF
{
  public:
    void init(int delayInSamples, double sampleRate)
    {
        fs = sampleRate;
        d.initDelay(delayInSamples, sampleRate);
        setFeedback(0.84);
        setDamping(0.2);
    }
    float getOutput(float input)
    {
        auto out = d.getOutput();

        auto lpOut = out * (1 - damping) + damping * state;
        state = lpOut;

        d.tick(input + lpOut * feedback);

        return out;
    }
    void setFeedback(float _f)
    {
        feedback = _f;
    }
    void setDamping(float _d)
    {
        damping = _d;
    }

  private:
    FixedDelayLine d;
    float state = 0.0f;
    float feedback = 0.84;
    float damping = 0.2;
    double fs = 48000;
};

// Special Freeverb Allpass filter
class FreeAP
{
  public:
    void init(int delayInSamples, double sampleRate)
    {
        fs = sampleRate;
        d.initDelay(delayInSamples, sampleRate);
    }
    float getOutput(float input)
    {
        auto dOut = d.getOutput();
        auto out = -input + dOut;

        d.tick(input + g * dOut);

        return out;
    }
    void setG(float _g)
    {
        g = _g;
    }

  private:
    FixedDelayLine d;
    float g = 0.5;
    double fs = 48000;
};

class Freeverb
{
  public:

    void setSampleRate(double sampleRate)
    {
        fs = sampleRate;
        auto conv = sampleRate / 44100.0;
        for (int i = 0; i < combAmount; i++)
            comb[i].init(round(combCoef[i] * conv), fs);

        for (int i = 0; i < apAmount; i++)
            ap[i].init(round(apCoef[i] * conv), fs);
    }

    float process(float input)
    {
        float out = 0.0f;

        // 8 in parallel
        for (int i = 0; i < combAmount; i++)
            out += comb[i].getOutput(input);

        // scale down
        out *= 0.125;
        // 4 in series
        for (int i = 0; i < apAmount; i++)
            out = ap[i].getOutput(out);

        return out;
    }
    void resetFilter()
    {
    }

    void setDamping(float _damping)
    {
        for (int i = 0; i < combAmount; i++)
            comb[i].setDamping(_damping);
    }

    void setFeedback(float _feedback)
    {
        for (int i = 0; i < combAmount; i++)
            comb[i].setFeedback(_feedback);
    }

  private:
    double fs = 48000;
    const static int combAmount = 8;
    const static int apAmount = 4;
    const int combCoef[combAmount] = {1557, 1617, 1491, 1422, 1277, 1356, 1188, 1116};
    const int apCoef[apAmount] = {225, 556, 441, 341};

    LBCF comb[combAmount];
    FreeAP ap[apAmount];
};