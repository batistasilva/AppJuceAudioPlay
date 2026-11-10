#include "MainComponent.h"

MainComponent::MainComponent()
{
    //Seta as configurações de audio
    setupAudio();

    //Seta os Labels
    setupLabels();

    //Seta os Sliders
    setupSliders();

    //Seta os Botões
    setupButtons();

    // ========== TAMANHO INICIAL ==========
    setSize(800, 500);

}


MainComponent::~MainComponent()
{
    // Para o timer antes de destruir
    stopTimer();
    
    // Libera o áudio (importante!)
    shutdownAudio();
}


// ========== MÉTODOS DE CONFIGURAÇÃO ==========

/**
 * @brief MainComponent::setupLabels
 */
void MainComponent::setupLabels()
{
    // ========== TÍTULO ==========
    titleLabel.setText("App Juce Audio Player", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // ========== LABELS DE TEMPO ==========
    timeCurrentLabel.setText("00:00", juce::dontSendNotification);
    timeCurrentLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(timeCurrentLabel);

    timeTotalLabel.setText("00:00", juce::dontSendNotification);
    timeTotalLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(timeTotalLabel);

    // ========== STATUS ==========
    statusLabel.setText("Nenhum arquivo carregado", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(statusLabel);

}

/**
 * @brief MainComponent::setupSliders
 */
void MainComponent::setupSliders()
{
    // ========== SLIDER DE VOLUME ==========
    volumeLabel.setText("Volume", juce::dontSendNotification);
    addAndMakeVisible(volumeLabel);

    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8);
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    volumeSlider.onValueChange = [this]() {
        audioPlayer.setVolume((float)volumeSlider.getValue());
    };
    addAndMakeVisible(volumeSlider);
}

/**
 * @brief MainComponent::setupButtons
 */
void MainComponent::setupButtons()
{
    // Define a cor de fundo do botão (um azul escuro elegante, combinando com o tema)
    playPauseButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    rewindButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    forwardButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    openButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));

    // ========== BOTÕES DE CONTROLE ==========
    openButton.onClick = [this]() { openFile(); };
    addAndMakeVisible(openButton);

    playPauseButton.onClick = [this]() {
        audioPlayer.togglePlayPause();
        updatePlaybackState();
    };
    addAndMakeVisible(playPauseButton);

    stopButton.onClick = [this]() {
        audioPlayer.stop();
        updatePlaybackState();
    };
    addAndMakeVisible(stopButton);

    rewindButton.onClick = [this]() {
        audioPlayer.seekToPosition(audioPlayer.getCurrentPosition() - 5.0);
    };
    addAndMakeVisible(rewindButton);

    forwardButton.onClick = [this]() {
        audioPlayer.seekToPosition(audioPlayer.getCurrentPosition() + 5.0);
    };
    addAndMakeVisible(forwardButton);
}

/**
 * @brief MainComponent::setupAudio
 */
void MainComponent::setupAudio()
{
    // ========== CONFIGURAÇÃO DO ÁUDIO ==========
    // Inicializa o áudio (0 entradas, 2 saídas)
    // 0 inputs, 2 outputs (estéreo)
    setAudioChannels(0, 2);

    // Inicia o timer para atualizar a UI (30 vezes por segundo)
    startTimerHz(30);
}



// ========== CICLO DE VIDA DO ÁUDIO ==========
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioPlayer.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    audioPlayer.getNextAudioBlock(bufferToFill);
}

void MainComponent::releaseResources()
{
    audioPlayer.releaseResources();
}

// ========== UI ==========
void MainComponent::paint(juce::Graphics& g)
{
    // Fundo escuro (estilo do projeto original)
    g.fillAll(juce::Colour(0xff1e1e2e));
    
    // Linha decorativa
    g.setColour(juce::Colour(0xff3a3a5a));
    g.drawLine(20, 60, getWidth() - 20, 60, 2.0f);
}


void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);

    // ========== TÍTULO ==========
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(20); // espaço

    // ========== SLIDER DE POSIÇÃO ==========
    auto positionRow = area.removeFromTop(30);
    timeCurrentLabel.setBounds(positionRow.removeFromLeft(60));
    timeTotalLabel.setBounds(positionRow.removeFromRight(60));
    positionSlider.setBounds(positionRow);

    area.removeFromTop(15);

    // ========== BOTÕES DE CONTROLE CENTRALIZADOS ==========
    auto buttonRow = area.removeFromTop(50);

    const int btnWidth = 80;
    const int spacing = 10;
    const int numButtons = 4;

    // Calcula a largura total que os 4 botões + 3 espaços ocupam
    int totalButtonsWidth = (btnWidth * numButtons) + (spacing * (numButtons - 1));

    // Calcula a margem para centralizar
    int margin = (buttonRow.getWidth() - totalButtonsWidth) / 2;

    // Se a tela for muito pequena, evita margem negativa
    margin = juce::jmax(0, margin);

    // Remove o espaço da esquerda para começar no centro
    buttonRow.removeFromLeft(margin);

    // Posiciona os botões
    rewindButton.setBounds(buttonRow.removeFromLeft(btnWidth));
    buttonRow.removeFromLeft(spacing);
    playPauseButton.setBounds(buttonRow.removeFromLeft(btnWidth));
    buttonRow.removeFromLeft(spacing);
    stopButton.setBounds(buttonRow.removeFromLeft(btnWidth));
    buttonRow.removeFromLeft(spacing);
    forwardButton.setBounds(buttonRow.removeFromLeft(btnWidth));

    area.removeFromTop(20);

    // ========== VOLUME ==========
    auto volumeRow = area.removeFromTop(30);
    volumeLabel.setBounds(volumeRow.removeFromLeft(70));
    volumeSlider.setBounds(volumeRow);

    area.removeFromTop(20);

    // ========== BOTÃO DE ABRIR + STATUS ==========
    auto openRow = area.removeFromTop(40);
    openButton.setBounds(openRow.removeFromLeft(150));
    openRow.removeFromLeft(20);
    statusLabel.setBounds(openRow);
}

void MainComponent::timerCallback()
{
    // Atualiza slider de posição se estiver tocando
    if (audioPlayer.isPlaying())
    {
        double pos = audioPlayer.getCurrentPosition();
        double len = audioPlayer.getLengthInSeconds();
        
        // Atualiza slider (sem triggar onValueChange)
        positionSlider.setValue(audioPlayer.getCurrentPositionFraction(), 
                                juce::dontSendNotification);
        
        // Atualiza labels de tempo
        timeCurrentLabel.setText(formatTime(pos), juce::dontSendNotification);
        timeTotalLabel.setText(formatTime(len), juce::dontSendNotification);
    }
}

// ========== HELPERS ==========
void MainComponent::openFile()
{
    // Diálogo de seleção de arquivo
    fileChooser = std::make_unique<juce::FileChooser>(
        juce::String::fromUTF8("Escolha um arquivo de áudio"), // <-- Força UTF-8
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.flac;*.ogg"
    );

    // Modo assíncrono (não bloqueia a UI)
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file.existsAsFile())
            {
                if (audioPlayer.loadFile(file))
                {
                    statusLabel.setText("Carregado: " + file.getFileName(),
                                       juce::dontSendNotification);
                    statusLabel.setColour(juce::Label::textColourId, 
                                         juce::Colours::lightgreen);
                    
                    // Atualiza duração total
                    timeTotalLabel.setText(
                        formatTime(audioPlayer.getLengthInSeconds()),
                        juce::dontSendNotification
                    );
                    
                    updatePlaybackState();
                }
                else
                {
                    statusLabel.setText("Erro ao carregar arquivo",
                                       juce::dontSendNotification);
                    statusLabel.setColour(juce::Label::textColourId,
                                         juce::Colours::red);
                }
            }
        }
    );
}

void MainComponent::updatePlaybackState()
{
    if (audioPlayer.isPlaying())
    {
        playPauseButton.setButtonText("Pause");
    }
    else
    {
        playPauseButton.setButtonText("Play");
    }
}

juce::String MainComponent::formatTime(double seconds)
{
    if (seconds < 0) seconds = 0;
    int totalSeconds = (int)seconds;
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;
    return juce::String(minutes).paddedLeft('0', 2) + ":" +
           juce::String(secs).paddedLeft('0', 2);
}