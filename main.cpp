#include <iostream>
#include <vector>
#include <portaudio.h>
#include <SFML/Graphics.hpp>
#include <fftw3.h>

using namespace std;

constexpr int SAMPLE_RATE = 44100;
constexpr int BUFFER_SIZE = 1024;

static int audioCallback(const void* inputBuffer, void* outputBuffer, unsigned long samplesPerFrame, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    auto* buffer = static_cast<vector<double>*>(userData);
    const float* buffer_values = static_cast<const float*>(inputBuffer);

    buffer->assign(buffer_values, buffer_values+samplesPerFrame);
    return paContinue;
}

int main() {



    return 0;
}
