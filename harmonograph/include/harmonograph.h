#pragma once
#include <cmath>

class Pendulum{
    private:
    double amplitude;
    double frequency;
    double phase;
    double damping;

    public:
    Pendulum(double a=1.0, double f=2.0, double p=0.0, double d =0.001) : amplitude(a), frequency(f), phase(p), damping(d) {}


    double calculate(double t) const {
        // position(t) = amplitude × e^(-damping × t) × sin(frequency × t + phase)
        return amplitude * std::exp(-damping * t) * std::sin(frequency * t + phase);
    }

    void setAmplitude(double a) { amplitude = a;}
    void setFrequency(double f) { frequency = f;}
    void setPhase(double p) { phase = p; }
    void setDamping(double d) { damping = d; }
    
    double getAmplitude() const { return amplitude; }
    double getFrequency() const { return frequency; }
    double getPhase() const { return phase; }
    double getDamping() const { return damping; }
};