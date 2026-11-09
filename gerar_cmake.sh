#!/usr/bin/env bash
# Gera um CMakeLists.txt para um projeto JUCE.
# Execute este script na raiz do projeto, ao lado da pasta Source.

set -euo pipefail

PROJECT_ROOT="$(pwd)"
SOURCE_DIR="${PROJECT_ROOT}/Source"
OUTPUT_FILE="${PROJECT_ROOT}/CMakeLists.txt"

echo "=== Gerador de CMake para projeto JUCE ==="
echo "Raiz do projeto: ${PROJECT_ROOT}"
echo

read -r -p "Nome do projeto: " PROJECT_DISPLAY_NAME
if [[ -z "${PROJECT_DISPLAY_NAME//[[:space:]]/}" ]]; then
    echo "Erro: informe o nome do projeto." >&2
    exit 1
fi

# O target do CMake deve usar caracteres adequados para identificadores.
PROJECT_TARGET="$(printf '%s' "$PROJECT_DISPLAY_NAME" |
    sed -E 's/[^A-Za-z0-9_]+/_/g; s/^_+//; s/_+$//')"

if [[ -z "$PROJECT_TARGET" ]]; then
    echo "Erro: não foi possível formar um nome válido para o target CMake." >&2
    exit 1
fi
if [[ "$PROJECT_TARGET" =~ ^[0-9] ]]; then
    PROJECT_TARGET="_${PROJECT_TARGET}"
fi

read -r -p "Caminho completo da pasta do JUCE: " JUCE_DIR
JUCE_DIR="${JUCE_DIR%\"}"
JUCE_DIR="${JUCE_DIR#\"}"
JUCE_DIR="${JUCE_DIR%\'}"
JUCE_DIR="${JUCE_DIR#\'}"

if [[ -z "$JUCE_DIR" ]]; then
    echo "Erro: informe o caminho da pasta do JUCE." >&2
    exit 1
fi

JUCE_DIR="${JUCE_DIR/#\~/$HOME}"
if [[ ! -d "$JUCE_DIR" || ! -f "$JUCE_DIR/CMakeLists.txt" ]]; then
    echo "Erro: informe a raiz da pasta JUCE, contendo CMakeLists.txt." >&2
    exit 1
fi

if [[ ! -d "$SOURCE_DIR" ]]; then
    echo "Erro: pasta Source não encontrada em ${PROJECT_ROOT}." >&2
    echo "Execute o script na raiz do projeto JUCE." >&2
    exit 1
fi

# Localiza arquivos de código recursivamente em Source.
mapfile -d '' SOURCE_FILES < <(
    find "$SOURCE_DIR" -type f \
        \( -iname '*.cpp' -o -iname '*.c' -o -iname '*.cc' -o -iname '*.cxx' \
           -o -iname '*.h' -o -iname '*.hh' -o -iname '*.hpp' -o -iname '*.hxx' \
           -o -iname '*.ipp' -o -iname '*.inl' -o -iname '*.mm' -o -iname '*.m' \) \
        -not -path '*/.*/*' -print0 | sort -z
)

if (( ${#SOURCE_FILES[@]} == 0 )); then
    echo "Erro: nenhum arquivo de código reconhecido foi encontrado em Source." >&2
    exit 1
fi

# Escapa barras invertidas e aspas para uso em uma string CMake.
JUCE_DIR_CMAKE="${JUCE_DIR//\\/\/}"
JUCE_DIR_CMAKE="${JUCE_DIR_CMAKE//\"/\\\"}"

if [[ -e "$OUTPUT_FILE" ]]; then
    read -r -p "CMakeLists.txt já existe. Sobrescrever? [s/N]: " OVERWRITE
    case "$OVERWRITE" in
        s|S|sim|SIM|Sim|y|Y|yes|YES|Yes) ;;
        *) echo "Cancelado; nenhum arquivo foi alterado."; exit 0 ;;
    esac
fi

# Cria o arquivo somente depois de validar as entradas e confirmar a sobrescrita.
{
    cat <<EOF
cmake_minimum_required(VERSION 3.22)
project(${PROJECT_TARGET} VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(JUCE_DIR "${JUCE_DIR_CMAKE}")
add_subdirectory("\${JUCE_DIR}" "\${CMAKE_BINARY_DIR}/JUCE")

juce_add_gui_app(${PROJECT_TARGET}
    VERSION 1.0.0
    PRODUCT_NAME "${PROJECT_DISPLAY_NAME//\"/\\\"}"
    NEEDS_WEB_BROWSER TRUE
)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set_target_properties(${PROJECT_TARGET} PROPERTIES
        JUCE_ANDROID_PACKAGE_NAME "com.exemplo.${PROJECT_TARGET}"
        JUCE_ANDROID_MIN_SDK_VERSION 29
        JUCE_ANDROID_TARGET_SDK_VERSION 34
    )
endif()

juce_generate_juce_header(${PROJECT_TARGET})

target_sources(${PROJECT_TARGET}
    PRIVATE
EOF

    for file in "${SOURCE_FILES[@]}"; do
        relative_path="${file#"$PROJECT_ROOT"/}"
        relative_path="${relative_path//\\/\/}"
        printf '        "%s"\n' "${relative_path//\"/\\\"}"
    done

    cat <<EOF
)

target_compile_definitions(${PROJECT_TARGET}
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_APPLICATION_NAME_STRING="\$<TARGET_PROPERTY:${PROJECT_TARGET},JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="\$<TARGET_PROPERTY:${PROJECT_TARGET},JUCE_VERSION>"
        JUCE_USE_MP3AUDIOFORMAT=1
)

target_link_libraries(${PROJECT_TARGET}
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.1)
    target_include_directories(${PROJECT_TARGET} PRIVATE \${WEBKIT_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_TARGET} PRIVATE \${WEBKIT_LIBRARIES})
endif()
EOF
} > "$OUTPUT_FILE"

echo
echo "CMakeLists.txt gerado: ${OUTPUT_FILE}"
echo "Target do projeto: ${PROJECT_TARGET}"
echo "Arquivos encontrados em Source: ${#SOURCE_FILES[@]}"
for file in "${SOURCE_FILES[@]}"; do
    printf '  - %s\n' "${file#"$PROJECT_ROOT"/}"
done
echo
echo "Pacote Android padrão: com.exemplo.${PROJECT_TARGET}"
echo "Edite esse identificador se quiser usar o pacote definitivo da aplicação."
