#include "MainComponent.h"

// ============================================================================
// CONSTRUTOR
// ============================================================================
// O construtor é responsável por CRIAR e CONFIGURAR os componentes.
// Ele não posiciona (isso é papel do resized) nem desenha (isso é papel do paint).
// Aqui apenas dizemos: "exista, tenha essa cor, esse texto, esse callback".
// ============================================================================
MainComponent::MainComponent()
{
    // Configura o dispositivo de áudio e inicia o timer que atualiza a UI.
    // (Ver detalhes em setupAudio() mais abaixo)
    setupAudio();

    // Cria e configura todos os Labels (título, tempos, status).
    setupLabels();

    // Cria e configura os Sliders (volume).
    setupSliders();

    // Cria e configura os Botões (play, stop, abrir, etc).
    setupButtons();

    // Define o tamanho inicial da janela.
    // O resized() será chamado automaticamente logo depois com esse tamanho.
    setSize(800, 500);
}

// ============================================================================
// DESTRUTOR
// ============================================================================
// Chamado quando o componente é destruído. Precisamos parar o timer e
// desligar o áudio para evitar crashes ou vazamentos de memória.
// ============================================================================
MainComponent::~MainComponent()
{
    // Para o timer (senão ele tentaria atualizar uma UI que não existe mais).
    stopTimer();
    
    // Desliga o áudio e libera o dispositivo (importante no JUCE!).
    shutdownAudio();
}

// ============================================================================
// MÉTODOS DE CONFIGURAÇÃO (chamados pelo construtor)
// ============================================================================

/**
 * @brief Configura todos os Labels (textos estáticos e dinâmicos).
 *
 * Labels são componentes que exibem texto. Aqui definimos:
 * - O texto inicial (com dontSendNotification para não disparar callbacks).
 * - O alinhamento (centro, esquerda, direita).
 * - A cor do texto (quando necessário).
 * - Adicionamos à hierarquia com addAndMakeVisible().
 *
 * O posicionamento (setBounds) NÃO é feito aqui, e sim no resized().
 */
void MainComponent::setupLabels()
{
    // ========== TÍTULO ==========
    // Texto grande no topo da janela.
    titleLabel.setText("App Juce Audio Player", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold)); // fonte 24, negrito
    titleLabel.setJustificationType(juce::Justification::centred); // centralizado
    addAndMakeVisible(titleLabel); // torna visível e adiciona à hierarquia

    // ========== LABELS DE TEMPO ==========
    // Mostram o tempo atual (esquerda) e o tempo total (direita) do áudio.
    timeCurrentLabel.setText("00:00", juce::dontSendNotification);
    timeCurrentLabel.setJustificationType(juce::Justification::centredRight);
    addAndMakeVisible(timeCurrentLabel);

    timeTotalLabel.setText("00:00", juce::dontSendNotification);
    timeTotalLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(timeTotalLabel);

    // ========== STATUS ==========
    // Mostra mensagens como "Nenhum arquivo carregado", "Tocando...", "Erro...".
    // A cor inicial é cinza (estado neutro). A cor muda dinamicamente em openFile().
    statusLabel.setText("Nenhum arquivo carregado", juce::dontSendNotification);
    statusLabel.setJustificationType(juce::Justification::centred);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::grey);
    addAndMakeVisible(statusLabel);
}

/**
 * @brief Configura todos os Sliders (controles deslizantes).
 *
 * Aqui configuramos:
 * - O range (mínimo, máximo, passo).
 * - O estilo visual (horizontal, vertical, rotary).
 * - O callback onValueChange (o que acontece quando o usuário move).
 * - O label associado (no caso do volume, um Label separado).
 *
 * O posicionamento (setBounds) fica no resized().
 */
void MainComponent::setupSliders()
{
    // ========== LABEL DO VOLUME ==========
    // Um Label separado que fica ao lado do slider de volume.
    volumeLabel.setText("Volume", juce::dontSendNotification);
    addAndMakeVisible(volumeLabel);

    // ========== SLIDER DE VOLUME ==========
    // Controla o volume do áudio (0.0 a 1.0, passo de 0.01).
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.8); // valor inicial (80%)
    volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal); // barra horizontal
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20); // caixa numérica à direita
    volumeSlider.onValueChange = [this]() {
        // Quando o usuário move o slider, chamamos o AudioPlayer.
        // Isso é seguro porque estamos na Message Thread (UI).
        audioPlayer.setVolume((float)volumeSlider.getValue());
    };
    addAndMakeVisible(volumeSlider);

    // NOTA: O positionSlider NÃO está aqui! Ele foi criado no tutorial original
    // mas no seu código atual ele não aparece no setupSliders.
    // Se você quiser adicioná-lo, precisa criar o membro no .h e configurá-lo aqui.
}

/**
 * @brief Configura todos os Botões (TextButton).
 *
 * Aqui configuramos:
 * - A cor de fundo (buttonColourId) para combinar com o tema escuro.
 * - O callback onClick (o que acontece quando o usuário clica).
 * - Adicionamos à hierarquia com addAndMakeVisible().
 *
 * O posicionamento (setBounds) fica no resized().
 */
void MainComponent::setupButtons()
{
    // ========== CORES DOS BOTÕES ==========
    // Define um azul escuro elegante (0xff2a2a4a) para todos os botões.
    // Sem isso, o JUCE usaria o cinza esverdeado padrão.
    playPauseButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    rewindButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    forwardButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));
    openButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a2a4a));

    // ========== BOTÃO ABRIR ARQUIVO ==========
    // Abre o FileChooser (diálogo nativo do sistema).
    // A lógica real está em openFile().
    openButton.onClick = [this]() { openFile(); };
    addAndMakeVisible(openButton);

    // ========== BOTÃO PLAY/PAUSE ==========
    // Alterna entre tocar e pausar. Atualiza o texto do botão.
    playPauseButton.onClick = [this]() {
        audioPlayer.togglePlayPause();
        updatePlaybackState(); // muda o texto para "Play" ou "Pause"
    };
    addAndMakeVisible(playPauseButton);

    // ========== BOTÃO STOP ==========
    // Para o áudio e volta ao início (posição 0).
    stopButton.onClick = [this]() {
        audioPlayer.stop();
        updatePlaybackState();
    };
    addAndMakeVisible(stopButton);

    // ========== BOTÃO RETROCEDER (-5s) ==========
    // Pula 5 segundos para trás na música.
    rewindButton.onClick = [this]() {
        audioPlayer.seekToPosition(audioPlayer.getCurrentPosition() - 5.0);
    };
    addAndMakeVisible(rewindButton);

    // ========== BOTÃO AVANÇAR (+5s) ==========
    // Pula 5 segundos para frente na música.
    forwardButton.onClick = [this]() {
        audioPlayer.seekToPosition(audioPlayer.getCurrentPosition() + 5.0);
    };
    addAndMakeVisible(forwardButton);
}

/**
 * @brief Configura o áudio e o timer.
 *
 * setAudioChannels(0, 2) inicializa o dispositivo de áudio:
 * - 0 entradas (não vamos gravar microfone neste tutorial).
 * - 2 saídas (estéreo).
 *
 * startTimerHz(30) inicia um timer que chama timerCallback() 30 vezes por segundo.
 * Usamos isso para atualizar o slider de posição e os labels de tempo.
 */
void MainComponent::setupAudio()
{
    // 0 inputs, 2 outputs (estéreo)
    setAudioChannels(0, 2);

    // Timer de 30 Hz (30 vezes por segundo)
    startTimerHz(30);
}

// ============================================================================
// CICLO DE VIDA DO ÁUDIO (chamados pela Audio Thread, NÃO pela UI Thread)
// ============================================================================
// Estes métodos são chamados pelo AudioDeviceManager. NUNCA chame eles
// manualmente e NUNCA toque em componentes de UI aqui dentro!
// ============================================================================

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // Prepara o AudioPlayer para tocar.
    // Aqui alocamos buffers internos (permitido, pois roda uma vez).
    audioPlayer.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Chamado repetidamente pela audio thread para preencher o buffer.
    // AQUI É PROIBIDO: alocar memória, tocar em UI, travar, fazer I/O.
    // Delegamos tudo para o AudioPlayer, que já está otimizado.
    audioPlayer.getNextAudioBlock(bufferToFill);
}

// ============================================================================
// CICLO DE VIDA DO ÁUDIO: releaseResources()
// ============================================================================
// Este método é o PARCEIRO de prepareToPlay(). Enquanto prepareToPlay()
// ALOCA recursos (buffers, filtros, memória), releaseResources() LIBERA
// esses recursos para evitar vazamentos de memória e crashes.
//
// QUEM CHAMA: O JUCE (via AudioDeviceManager), NÃO o seu código.
// QUANDO CHAMA:
//   - Ao fechar o app (shutdownAudio())
//   - Ao trocar de dispositivo de áudio (ex: plugar headphone)
//   - Ao mudar sample rate ou buffer size
//   - Ao reiniciar o áudio manualmente
//
// REGRA DE OURO: NUNCA toque em componentes de UI aqui!
// NUNCA aloque memória, NUNCA faça I/O, NUNCA trave com mutex.
// Só repasse a chamada para o AudioPlayer e deixe ele limpar o resto.
// ============================================================================
void MainComponent::releaseResources()
{
    // Delegamos a limpeza para o AudioPlayer.
    // O AudioPlayer sabe quais recursos internos precisa liberar
    // (TransportSource, buffers de leitura, efeitos DSP, etc.).
    // Isso mantém o encapsulamento: o MainComponent não precisa
    // saber COMO o AudioPlayer funciona por dentro.
    audioPlayer.releaseResources();
}

// ============================================================================
// UI: PAINT (desenho do fundo e decoração)
// ============================================================================
// Chamado quando o componente precisa ser redesenhado (30-60x por segundo).
// Aqui desenhamos APENAS o que NÃO é componente filho: fundo, linhas, formas.
// NUNCA crie componentes aqui, NUNCA aloque memória pesada.
// ============================================================================
void MainComponent::paint(juce::Graphics& g)
{
    // Fundo escuro (estilo do projeto original)
    g.fillAll(juce::Colour(0xff1e1e2e));
    
    // Linha decorativa abaixo do título
    g.setColour(juce::Colour(0xff3a3a5a));
    g.drawLine(20, 60, getWidth() - 20, 60, 2.0f);
}

// ============================================================================
// UI: RESIZED (posicionamento dos componentes)
// ============================================================================
// Chamado quando o componente muda de tamanho (inicialmente e quando o
// usuário redimensiona a janela). Aqui posicionamos os filhos com setBounds().
// NUNCA crie componentes aqui, NUNCA desenhe aqui.
// ============================================================================
void MainComponent::resized()
{
    // Área útil (com margem de 20px em todos os lados)
    auto area = getLocalBounds().reduced(20);

    // ========== TÍTULO ==========
    // Ocupa 40px de altura no topo.
    titleLabel.setBounds(area.removeFromTop(40));
    area.removeFromTop(20); // espaço vazio

    // ========== SLIDER DE POSIÇÃO + TEMPOS ==========
    // Linha de 30px de altura com:
    // [timeCurrentLabel] [positionSlider] [timeTotalLabel]
    auto positionRow = area.removeFromTop(30);
    timeCurrentLabel.setBounds(positionRow.removeFromLeft(60));  // 60px à esquerda
    timeTotalLabel.setBounds(positionRow.removeFromRight(60));   // 60px à direita
    positionSlider.setBounds(positionRow);                        // o resto no meio

    area.removeFromTop(15);

    // ========== BOTÕES DE CONTROLE CENTRALIZADOS ==========
    // Linha de 50px de altura para os 4 botões.
    auto buttonRow = area.removeFromTop(50);

    const int btnWidth = 80;   // largura de cada botão
    const int spacing = 10;    // espaço entre botões
    const int numButtons = 4;  // <<, Play, Stop, >>

    // Calcula a largura total ocupada pelos botões + espaços
    int totalButtonsWidth = (btnWidth * numButtons) + (spacing * (numButtons - 1));

    // Calcula a margem para centralizar
    int margin = (buttonRow.getWidth() - totalButtonsWidth) / 2;
    margin = juce::jmax(0, margin); // evita margem negativa em telas pequenas

    // Remove o espaço da esquerda para começar no centro
    buttonRow.removeFromLeft(margin);

    // Posiciona os botões em sequência
    rewindButton.setBounds(buttonRow.removeFromLeft(btnWidth));
    buttonRow.removeFromLeft(spacing);
    playPauseButton.setBounds(buttonRow.removeFromLeft(btnWidth));
    buttonRow.removeFromLeft(spacing);
    stopButton.setBounds(buttonRow.removeFromLeft(btnWidth));
    buttonRow.removeFromLeft(spacing);
    forwardButton.setBounds(buttonRow.removeFromLeft(btnWidth));

    area.removeFromTop(20);

    // ========== VOLUME ==========
    // Linha de 30px: [volumeLabel] [volumeSlider]
    auto volumeRow = area.removeFromTop(30);
    volumeLabel.setBounds(volumeRow.removeFromLeft(70));
    volumeSlider.setBounds(volumeRow);

    area.removeFromTop(20);

    // ========== BOTÃO DE ABRIR + STATUS ==========
    // Linha de 40px: [openButton] [statusLabel]
    auto openRow = area.removeFromTop(40);
    openButton.setBounds(openRow.removeFromLeft(150));
    openRow.removeFromLeft(20);
    statusLabel.setBounds(openRow);
}



// ============================================================================
// TIMER CALLBACK (atualiza a UI periodicamente)
// ============================================================================
// Chamado 30 vezes por segundo (definido em startTimerHz(30)).
// Aqui atualizamos o slider de posição e os labels de tempo enquanto toca.
// É seguro tocar em UI aqui (estamos na Message Thread).
// ============================================================================
void MainComponent::timerCallback()
{
    if (audioPlayer.isPlaying())
    {
        double pos = audioPlayer.getCurrentPosition();
        double len = audioPlayer.getLengthInSeconds();
        
        // Atualiza slider (sem triggar onValueChange)
        // dontSendNotification evita loop infinito (setValue -> onValueChange -> seek -> setValue...)
        positionSlider.setValue(audioPlayer.getCurrentPositionFraction(), 
                                juce::dontSendNotification);
        
        // Atualiza labels de tempo
        timeCurrentLabel.setText(formatTime(pos), juce::dontSendNotification);
        timeTotalLabel.setText(formatTime(len), juce::dontSendNotification);
    }
}

// ============================================================================
// HELPERS (funções auxiliares)
// ============================================================================

/**
 * @brief Abre o diálogo de seleção de arquivo.
 *
 * Usa juce::FileChooser em modo assíncrono (launchAsync) para não bloquear
 * a UI. Quando o usuário escolhe um arquivo, o callback é chamado.
 *
 * Se o arquivo for válido, carrega no AudioPlayer e atualiza o status.
 */
void MainComponent::openFile()
{
    // Cria o FileChooser com:
    // - Título da janela (forçado UTF-8 para evitar erro de acento)
    // - Diretório inicial (pasta de músicas do usuário)
    // - Filtro de extensões suportadas
    fileChooser = std::make_unique<juce::FileChooser>(
        juce::String::fromUTF8("Escolha um arquivo de áudio"),
        juce::File::getSpecialLocation(juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.flac;*.ogg"
    );

    // Abre o diálogo em modo assíncrono.
    // O lambda é chamado quando o usuário escolhe (ou cancela).
    fileChooser->launchAsync(
        juce::FileBrowserComponent::openMode |
        juce::FileBrowserComponent::canSelectFiles,
        [this](const juce::FileChooser& chooser)
        {
            auto file = chooser.getResult();
            if (file.existsAsFile())
            {
                // Tenta carregar no AudioPlayer
                if (audioPlayer.loadFile(file))
                {
                    // Sucesso: atualiza status (verde) e tempo total
                    statusLabel.setText("Carregado: " + file.getFileName(),
                                       juce::dontSendNotification);
                    statusLabel.setColour(juce::Label::textColourId, 
                                         juce::Colours::lightgreen);
                    
                    timeTotalLabel.setText(
                        formatTime(audioPlayer.getLengthInSeconds()),
                        juce::dontSendNotification
                    );
                    
                    updatePlaybackState();
                }
                else
                {
                    // Erro: mostra status em vermelho
                    statusLabel.setText("Erro ao carregar arquivo",
                                       juce::dontSendNotification);
                    statusLabel.setColour(juce::Label::textColourId,
                                         juce::Colours::red);
                }
            }
        }
    );
}

/**
 * @brief Atualiza o texto do botão Play/Pause.
 *
 * Chamado após togglePlayPause(), stop(), etc.
 * Se estiver tocando: "Pause". Se não: "Play".
 */
void MainComponent::updatePlaybackState()
{
    if (audioPlayer.isPlaying())
        playPauseButton.setButtonText("Pause");
    else
        playPauseButton.setButtonText("Play");
}

/**
 * @brief Formata segundos em "MM:SS".
 *
 * Ex: 125.7 -> "02:05"
 * Usado nos labels de tempo atual e total.
 */
juce::String MainComponent::formatTime(double seconds)
{
    if (seconds < 0) seconds = 0;
    int totalSeconds = (int)seconds;
    int minutes = totalSeconds / 60;
    int secs = totalSeconds % 60;
    return juce::String(minutes).paddedLeft('0', 2) + ":" +
           juce::String(secs).paddedLeft('0', 2);
}