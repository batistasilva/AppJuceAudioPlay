#pragma once
#include <JuceHeader.h>

/**
 * AudioPlayer - Motor de áudio do player
 * 
 * Responsabilidades:
 * - Carregar arquivos de áudio (WAV, MP3, FLAC, OGG)
 * - Controlar playback (play, stop, pause)
 * - Gerenciar volume
 * - Fornecer samples para a saída de áudio
 */
class AudioPlayer
{
public:
    AudioPlayer();
    ~AudioPlayer();

    // ========== CARREGAMENTO ==========
    /** Carrega um arquivo de áudio. Retorna true se sucesso. */
    bool loadFile(const juce::File& file);
    
    /** Verifica se há um arquivo carregado */
    bool isLoaded() const { return transportSource.getLengthInSeconds() > 0; }

    // ========== CONTROLE DE PLAYBACK ==========
    void play();
    void stop();
    void pause();
    void togglePlayPause();
    void seekToPosition(double positionInSeconds);
    void seekToFraction(float fraction); // 0.0 a 1.0

    // ========== ESTADO ==========
    bool isPlaying() const { return transportSource.isPlaying(); }
    double getCurrentPosition() const { return transportSource.getCurrentPosition(); }
    double getLengthInSeconds() const { return transportSource.getLengthInSeconds(); }
    float getCurrentPositionFraction() const;

    // ========== PARÂMETROS ==========
    void setVolume(float newVolume); // 0.0 a 1.0
    float getVolume() const { return volume; }

    // ========== CICLO DE VIDA DO ÁUDIO ==========
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate);
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);
    void releaseResources();

private:
    // Leitor de formatos (WAV, MP3, etc)
    juce::AudioFormatManager formatManager;
    
    // Fonte do arquivo atual
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    
    // Controlador de playback
    juce::AudioTransportSource transportSource;
    
    // Volume atual (0.0 a 1.0)
    float volume = 0.8f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPlayer)
};