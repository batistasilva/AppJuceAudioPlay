#pragma once
#include <JuceHeader.h>
#include "AudioPlayer.h"

/**
 * MainComponent - Interface gráfica principal
 * 
 * Herda de AudioAppComponent que gerencia automaticamente
 * o dispositivo de áudio (AudioDeviceManager).
 */
class MainComponent : public juce::AudioAppComponent,
                      public juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    // ========== CICLO DE VIDA DO ÁUDIO (AudioAppComponent) ==========
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // ========== UI ==========
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override; // Atualiza slider de posição

private:
    // ========== MOTOR DE ÁUDIO ==========
    AudioPlayer audioPlayer;

    // ========== COMPONENTES VISUAIS ==========
    juce::Label titleLabel;
    juce::Label timeCurrentLabel;
    juce::Label timeTotalLabel;
    juce::Label statusLabel;
    juce::Label volumeLabel;

    juce::Slider positionSlider;
    juce::Slider volumeSlider;

    juce::TextButton openButton { "Carregar Arquivo" };
    juce::TextButton playPauseButton { "Play" };
    juce::TextButton stopButton { "Stop" };
    juce::TextButton rewindButton { "<<" };
    juce::TextButton forwardButton { ">>" };

    // ========== DIÁLOGO DE ARQUIVO ==========
    std::unique_ptr<juce::FileChooser> fileChooser;

    // ========== HELPERS ==========
    void openFile();
    void updatePlaybackState();
    juce::String formatTime(double seconds);


private:
    void setupLabels();
    void setupSliders();
    void setupButtons();
    void setupAudio();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};