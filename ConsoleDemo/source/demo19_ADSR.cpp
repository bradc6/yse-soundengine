#include <iostream>
#include <cstdlib>
#include <cstdio>
#include "yse.hpp"

#ifdef YSE_WINDOWS
#include <conio.h>
#else
#include "wincompat.h"
#endif

/** ADSR envelopes:
    
    This is a combination of the envelope and an audio buffer. It can be applied to a
    normal audio buffer. In this example an FM synth is created with a wavetable and
    an envelope.

    Actually, this is not a real 'ADSR' envelope because it is not limited to four points.
    You can add as much points you like. If you want the sound to keep playing until
    it is recieves a note off, you'll have to mark the beginning and the end of a loop.
    The loop will keep repeating until a note off happens.

    The note off behaviour causes the envelope to stop looping. But since the current value
    in the envelope can be very different from where the release begins, it cannot just jump
    to the release point. Instead, it will jump to the last point in the looping part with the
    same value as the current value. So when the loop does not change the envelope, this will 
    be the release point. If the loop does change the envelope, position will jump to the point 
    closest to release.
*/

Bool tableReady = false;

class synthVoice : public YSE::SYNTH::dspVoice {
public:

  synthVoice() {
    generator.initialize(getTable());

    // addPoint assumes that all breakpoints are given in order of time
    envelope.addPoint(YSE::DSP::ADSRenvelope::breakPoint(0.f, 0.f, 0.2));
    envelope.addPoint(YSE::DSP::ADSRenvelope::breakPoint(0.1f, 1.f, 4));

    // next point is the beginning of the loop
    envelope.addPoint(YSE::DSP::ADSRenvelope::breakPoint(0.2f, 0.5f, 2, true));
    envelope.addPoint(YSE::DSP::ADSRenvelope::breakPoint(0.3f, 0.9f, 0.5));

    // next point is the end of the loop
    envelope.addPoint(YSE::DSP::ADSRenvelope::breakPoint(0.4f, 0.5f, 0.5, false, true));
    envelope.addPoint(YSE::DSP::ADSRenvelope::breakPoint(0.5f, 0.f, 0.5));
    envelope.generate();
  }

  virtual dspVoice * clone() {
    return new synthVoice();
  }

  virtual void process(YSE::SOUND_STATUS & intent) {
    YSE::DSP::ADSRenvelope::STATE state = YSE::DSP::ADSRenvelope::RESUME;

    if (intent == YSE::SS_WANTSTOPLAY) {
      // this indicates the start of a new note
      state = YSE::DSP::ADSRenvelope::ATTACK;

      // notify backend that the note has started
      intent = YSE::SS_PLAYING;
      releaseRequested = false;
    }
    else if (intent == YSE::SS_WANTSTOSTOP && !releaseRequested) {
      // backend would like to stop this note
      state = YSE::DSP::ADSRenvelope::RELEASE;
      releaseRequested = true;
    }

    
    out = generator(getFrequency());
    out *= envelope(state); // this updates the envelope and aplies it to the output buffer
    out *= 0.25f;
    
    // notify the backend that this note has stopped playing
    if (envelope.isAtEnd()) intent = YSE::SS_STOPPED;

    for (UInt i = 0; i < buffer.size(); i++) {
      buffer[i] = out;
    }
    
  }

private:
  YSE::DSP::oscillator generator;
  YSE::DSP::ADSRenvelope envelope;
  YSE::DSP::buffer out;
  bool releaseRequested;

  static YSE::DSP::wavetable & getTable() {
    static YSE::DSP::wavetable  table;
    if (!tableReady) {
      table.createTriangle(8, 1024);
      tableReady = true;
    }
    return table;
  }

};

YSE::synth synth;
YSE::sound sound;

Int bassNote, middleNote;

int main() {
  YSE::System().init();

  std::cout << "press e to exit." << std::endl;

  synth.create();
  synthVoice voice;
  synth.addVoices(&voice, 8, 1);
  sound.create(synth).play();

  int counter = 0;
  while (true) {

    // random player
    if (YSE::Random(10) == 0) {
      synth.noteOff(1, bassNote);
      bassNote = YSE::Random(30, 50);
      Flt vel = YSE::RandomF(0.8, 0.9);
      synth.noteOn(1, bassNote, vel);
    }

    // melody
    if (YSE::Random(6) == 0) {
      synth.noteOff(1, middleNote);
      synth.noteOff(1, middleNote - 3);
      middleNote = YSE::Random(55, 65);
      synth.noteOn(1, middleNote, YSE::RandomF(0.5, 0.7));
      synth.noteOn(1, middleNote - 3, YSE::RandomF(0.5, 0.7));
    }

    counter++;

    if (_kbhit()) {
      char ch = _getch();
      switch (ch) {


      case 'e': goto exit;
      }
    }
    YSE::System().sleep(100);
    YSE::System().update();
  }


exit:
  YSE::System().close();
  return 0;
}