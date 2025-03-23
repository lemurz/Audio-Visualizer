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

    while(window.isOpen()){

        while (const optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();

        fftw_execute(plan);

        const size_t totalBars = BUFFER_SIZE / 2;
        const float barWidth = 3.0f;
        const float barSpacing = 1.0f;

        const float scaleFactorX = WINDOW_WIDTH / (totalBars * (barWidth + barSpacing));

        const float adjustedBarWidth = barWidth * scaleFactorX;
        const float adjustedBarSpacing = barSpacing * scaleFactorX;
        const float scaleFactor = 5.0f;

        for(size_t i = 0; i < BUFFER_SIZE/2; i++){
            float magnitude = sqrt(static_cast<float>(fftOutput[i][0] * fftOutput[i][0] + fftOutput[i][1] * fftOutput[i][1]));
            //float logMagnitude = log(1.0f + magnitude) * 10.0f;

            float frequencyScale = 1.0f + (static_cast<float>(i) / (BUFFER_SIZE / 4));
            float scaledMagnitude = magnitude * scaleFactor * frequencyScale;

            sf::RectangleShape bar(sf::Vector2f(adjustedBarWidth, scaledMagnitude));
            bar.setFillColor(sf::Color(100 + i % 156, 50 + i % 206, 150 + i % 106));

            float xPos = i * (adjustedBarWidth + adjustedBarSpacing);
            float yPos = 600 - bar.getSize().y;

            bar.setPosition({xPos, yPos});
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
