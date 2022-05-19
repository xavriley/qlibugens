//
// Created by Xavier Riley on 19/05/2022.
//

#include "ScaleMapper.h"

const float ScaleMapper::map_note_to_musical_scale(int chosen_key, std::vector<ToneRange> chosen_scale, float midi_in) {
    // get the midiIn float in the range 0-12
    int midi_in_int = int(floorf(midi_in));

    // TODO: how to minimise octave jumps when using different key bases?
    float leftover = ((midi_in_int % 12) + chosen_key) + (midi_in - midi_in_int);

    // lookup for ranges adapted from https://stackoverflow.com/a/42510914/2618015
    auto it = std::find_if(
            chosen_scale.begin(),
            chosen_scale.end(),
            [&leftover](ToneRange tr) { return (tr.lower <= leftover) && (tr.upper >= leftover); }
    );

    if (it == chosen_scale.end()) {
        // not found - should never happen unless the scales are defined wrong
        return midi_in;
    } else {
        // this gets the root of the current key then adds the mapped note
        return midi_in - leftover + it->out;
    }
}
