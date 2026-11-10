#include "AudioPlayer.h"

AudioPlayer::AudioPlayer()
{
    // Registra os formatos suportados
    formatManager.registerBasicFormats(); // WAV, AIFF, MP3, OGG, FLAC
    
    // Conecta o transport source ao mixer
    // (o transport source será conectado ao device manager no MainComponent)
}

AudioPlayer::~AudioPlayer()
{
    // Libera recursos antes de destruir
    transportSource.setSource(nullptr);
    readerSource.reset();
}

// ========== CARREGAMENTO ==========
bool AudioPlayer::loadFile(const juce::File& file)
{
    // Para qualquer playback atual
    transportSource.stop();
    transportSource.setSource(nullptr);
    readerSource.reset();

    // Verifica se o arquivo existe
    if (!file.existsAsFile())
    {
        DBG("Arquivo não encontrado: " + file.getFullPathName());
        return false;
    }

    // Cria um leitor para o arquivo
    std::unique_ptr<juce::AudioFormatReader> reader(
        formatManager.createReaderFor(file)
    );

    if (reader == nullptr)
    {
        DBG("Formato não suportado: " + file.getFullPathName());
        return false;
    }

    // Cria a fonte de leitura
    readerSource = std::make_unique<juce::AudioFormatReaderSource>(
        reader.release(), true // deleteReaderWhenDone
    );

    // Conecta ao transport source
    transportSource.setSource(
        readerSource.get(),
        0,                    // readAheadBufferSize (0 = padrão)
        nullptr,              // thread para background loading
        readerSource->getAudioFormatReader()->sampleRate
    );

    return true;
}

// ========== CONTROLE DE PLAYBACK ==========
void AudioPlayer::play()
{
    if (isLoaded())
        transportSource.start();
}

void AudioPlayer::stop()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
}

void AudioPlayer::pause()
{
    transportSource.stop();
}

void AudioPlayer::togglePlayPause()
{
    if (isPlaying())
        pause();
    else
        play();
}

void AudioPlayer::seekToPosition(double positionInSeconds)
{
    if (isLoaded())
        transportSource.setPosition(positionInSeconds);
}

void AudioPlayer::seekToFraction(float fraction)
{
    if (isLoaded())
    {
        fraction = juce::jlimit(0.0f, 1.0f, fraction);
        transportSource.setPosition(fraction * getLengthInSeconds());
    }
}

// ========== ESTADO ==========
float AudioPlayer::getCurrentPositionFraction() const
{
    double length = getLengthInSeconds();
    if (length <= 0.0) return 0.0f;
    return (float)(getCurrentPosition() / length);
}

// ========== PARÂMETROS ==========
void AudioPlayer::setVolume(float newVolume)
{
    volume = juce::jlimit(0.0f, 1.0f, newVolume);
    // Aplica ganho no transport source
    transportSource.setGain(volume);
}

// ========== CICLO DE VIDA DO ÁUDIO ==========
void AudioPlayer::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void AudioPlayer::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Limpa o buffer (silêncio)
    bufferToFill.clearActiveBufferRegion();
    
    // Preenche com samples do transport source
    transportSource.getNextAudioBlock(bufferToFill);
}

void AudioPlayer::releaseResources()
{
    transportSource.releaseResources();
}