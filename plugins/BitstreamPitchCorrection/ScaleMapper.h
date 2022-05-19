#ifndef SCALEMAPPER_H_INCLUDED
#define SCALEMAPPER_H_INCLUDED

#include <map>
#include <vector>

class ScaleMapper {
public:
    struct ToneRange { float lower; float upper; int out; };

    const static inline std::map<int, int> keys {
        {0, 0},
        {1, -1},
        {2, -2},
        {3, -3},
        {4, -4},
        {5, -5},
        {6, -6},
        {7, 5},
        {8, 4},
        {9, 3},
        {10, 2},
        {11, 1},
    };

    const static inline std::vector<ToneRange> chromatic = {
            {0.0f, 0.5f, 0},
            {0.5f, 1.5f, 1},
            {1.5f, 2.5f, 2},
            {2.5f, 3.5f, 3},
            {3.5f, 4.5f, 4},
            {4.5f, 5.5f, 5},
            {5.5f, 6.5f, 6},
            {6.5f, 7.5f, 7},
            {7.5f, 8.5f, 8},
            {8.5f, 9.5f, 9},
            {9.5f, 10.5f, 10},
            {10.5f, 11.5f, 11},
            {11.5f, 12.0f, 12}
    };

    const static inline std::vector<ToneRange> major = {
            {0.0f, 1.0f, 0},
            {1.0f, 3.0f, 2},
            {3.0f, 4.5f, 4},
            {4.5f, 6.0f, 5},
            {6.0f, 8.0f, 7},
            {8.0f, 10.0f, 9},
            {10.0f, 12.0f, 11}
    };

    const static inline std::vector<ToneRange> major_pentatonic = {
            {0.0f, 1.0f, 0},
            {1.0f, 3.0f, 2},
            {3.0f, 5.5f, 4},
            {5.5f, 8.0f, 7},
            {8.0f, 10.0f, 9},
            {10.0f, 12.0f, 12}
    };

    const static inline std::vector<ToneRange> minor_pentatonic = {
            {0.0f, 1.5f, 0},
            {1.5f, 4.0f, 3},
            {4.0f, 6.0f, 5},
            {6.0f, 8.5f, 7},
            {8.5f, 11.0f, 10},
            {11.0f, 12.0f, 12}
    };

    const static inline std::vector<std::vector<ToneRange>> key_picker = {
            chromatic,
            major,
            major_pentatonic,
            minor_pentatonic,
    };

    const static float map_note_to_musical_scale(int chosen_key, std::vector<ToneRange> chosen_scale, float midi_in);
};

#endif
