/*
  ==============================================================================

    difference.cpp
    Created: 4 Aug 2014 1:21:47pm
    Author:  yvan

  ==============================================================================
*/

#include "difference.hpp"

YSE::DSP::MODULES::difference::difference() : parmFrequency(400.f), parmAmplitude(0.5f) {}

YSE::DSP::MODULES::difference & YSE::DSP::MODULES::difference::frequency(Flt value) {
  parmFrequency.store(value);
  return *this;
}

Flt YSE::DSP::MODULES::difference::frequency() {
  return parmFrequency;
}

YSE::DSP::MODULES::difference & YSE::DSP::MODULES::difference::amplitude(Flt value) {
  parmAmplitude.store(value);
  return *this;
}

Flt YSE::DSP::MODULES::difference::amplitude() {
  return parmAmplitude;
}

void YSE::DSP::MODULES::difference::create() {
  source.reset(new sine);
  clipper.reset(new clip);
  clipper->set(-1, 1);
}

void YSE::DSP::MODULES::difference::process(MULTICHANNELBUFFER & buffer) {
  createIfNeeded();

  for (UInt i = 0; i < buffer.size(); i++) {
    buffer[i] += (*source)(parmFrequency);
    buffer[i] *= parmAmplitude.load();
    buffer[i] = (*clipper)(buffer[i]);
  }
}