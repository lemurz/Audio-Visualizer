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

    vector<double> audioBuffer(BUFFER_SIZE);
    PaStream* stream;
    Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, BUFFER_SIZE, audioCallback, &audioBuffer);
    Pa_StartStream(stream);

    fftw_complex* fftOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * BUFFER_SIZE);
    fftw_plan plan = fftw_plan_dft_r2c_1d(BUFFER_SIZE, audioBuffer.data(), fftOutput, FFTW_ESTIMATE);

    sf::RenderWindow window(sf::VideoMode(800, 600), "Audio Spectrum Visualizer");

    return 0;
}
