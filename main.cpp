#include <iostream>
#include <vector>
#include <portaudio.h>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <fftw3.h>
#include <cmath>
#include <optional>

using namespace std;

constexpr int SAMPLE_RATE = 44100;
constexpr int BUFFER_SIZE = 1024;
constexpr unsigned WINDOW_HEIGHT = 600u;
constexpr unsigned WINDOW_WIDTH = 800u;
constexpr size_t DISPLAY_BARS = 64;
constexpr float SMOOTHING_FACTOR = 0.85f;

static int audioCallback(const void* inputBuffer, void* outputBuffer, unsigned long samplesPerFrame, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) {
    auto* buffer = static_cast<vector<double>*>(userData);
    const auto* buffer_values = static_cast<const float*>(inputBuffer);

    buffer->assign(buffer_values, buffer_values+samplesPerFrame);
    return paContinue;
}

int main() {

    PaError err = Pa_Initialize();

    if (err != paNoError) {
        cout<<"Port audio error : "<<Pa_GetErrorText(err)<<endl;
        return 1;
    }

    vector<double> audioBuffer(BUFFER_SIZE);
    PaStream* stream;

    err = Pa_OpenDefaultStream(&stream, 1, 0, paFloat32, SAMPLE_RATE, BUFFER_SIZE, audioCallback, &audioBuffer);

    if (err != paNoError) {
        cout<<"Port audio error : "<<Pa_GetErrorText(err)<<endl;
        Pa_Terminate();
        return 1;
    }

    Pa_StartStream(stream);

    auto* fftOutput = static_cast<fftw_complex*> (fftw_malloc(sizeof(fftw_complex) * BUFFER_SIZE));
    auto* plan = fftw_plan_dft_r2c_1d(BUFFER_SIZE, audioBuffer.data(), fftOutput, FFTW_ESTIMATE);

    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "Audio Spectrum Visualizer");
    window.setFramerateLimit(60);

    vector<float> smoothedMagnitudes(DISPLAY_BARS, 0.0f);

    while(window.isOpen()){

        while (const optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();

        fftw_execute(plan);

        const double freqMin = 20.0;
        const double freqMax =  SAMPLE_RATE / 2.0;

        for(size_t i = 0; i < DISPLAY_BARS; i++){

            double fractionLow = static_cast<double>(i) / DISPLAY_BARS;
            double fractionHigh = static_cast<double>(i + 1) / DISPLAY_BARS;

            double freqLow = freqMin * pow(freqMax/freqMin, fractionLow);
            double freqHigh = freqMin * pow(freqMax/freqMin, fractionHigh);

            int binLow = static_cast<int>(freqLow * BUFFER_SIZE / SAMPLE_RATE);
            int binHigh = static_cast<int>(freqHigh * BUFFER_SIZE / SAMPLE_RATE);

            if (binHigh <= binLow) {
                binHigh = binLow + 1;
            }

            double sum = 0.0;

            for (int i = binLow; i < binHigh && i < BUFFER_SIZE / 2; i++) {
                float real = fftOutput[i][0];
                float imag = fftOutput[i][1];
                sum += sqrt(real * real + imag * imag);
            }

            double avgMagnitude = sum / (binHigh - binLow);
            smoothedMagnitudes[i] = SMOOTHING_FACTOR * avgMagnitude + (1.0f - SMOOTHING_FACTOR) * smoothedMagnitudes[i];

            float barWidth = static_cast<float>(WINDOW_WIDTH) / DISPLAY_BARS;
            float barHeight = smoothedMagnitudes[i] * 5.0f;
            barHeight = min(barHeight, static_cast<float>(WINDOW_HEIGHT));

            sf::RectangleShape bar(sf::Vector2f(barWidth - 2.0f, -barHeight));
            bar.setPosition({i * barWidth, WINDOW_HEIGHT});
            bar.setFillColor(sf::Color(180, 100 + (i * 2) % 155, 200));

            window.draw(bar);
        }

        window.display();

    }

    fftw_destroy_plan(plan);
    fftw_free(fftOutput);
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    return 0;
}
