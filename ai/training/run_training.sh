#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Default parameters
TIMESTEPS=50000
WIDTH=10
HEIGHT=10
FREQ=100
TEAM="TeamAI"
MODEL_NAME="zappy_ai_model"
LOAD_MODEL=""
DEBUG_BUILD=false
ENVS=""

# Help message
print_usage() {
    echo "Usage: ./run_training.sh [OPTIONS]"
    echo "Options:"
    echo "  -t, --timesteps N   Total timesteps to train (default: $TIMESTEPS)"
    echo "  -x, --width N       Map width (default: $WIDTH)"
    echo "  -y, --height N      Map height (default: $HEIGHT)"
    echo "  -f, --freq N        Simulation frequency (default: $FREQ)"
    echo "  -n, --team NAME     Team name (default: $TEAM)"
    echo "  -m, --model NAME    Saved model name (default: $MODEL_NAME)"
    echo "  -l, --load-model NAME Model name/path to load and continue training"
    echo "  -e, --envs N        Number of parallel environments (default: half of CPU cores)"
    echo "  -d, --debug         Build Rust library in debug mode instead of release"
    echo "  -h, --help          Show this help message"
}

# Parse command line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -t|--timesteps) TIMESTEPS="$2"; shift ;;
        -x|--width) WIDTH="$2"; shift ;;
        -y|--height) HEIGHT="$2"; shift ;;
        -f|--freq) FREQ="$2"; shift ;;
        -n|--team) TEAM="$2"; shift ;;
        -m|--model) MODEL_NAME="$2"; shift ;;
        -l|--load-model) LOAD_MODEL="$2"; shift ;;
        -e|--envs) ENVS="$2"; shift ;;
        -d|--debug) DEBUG_BUILD=true ;;
        -h|--help) print_usage; exit 0 ;;
        *) echo "Unknown parameter passed: $1"; print_usage; exit 1 ;;
    esac
    shift
done

# Navigate to project root (2 directories up from ai/training/)
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/../.."
PROJECT_ROOT="$(pwd)"

echo "==================================="
echo " BUILDING ZAPPY ENGINE LIBRARY"
echo "==================================="
if [ "$DEBUG_BUILD" = true ]; then
    echo "Building in debug mode..."
    cargo build -q --manifest-path=server/Cargo.toml
else
    echo "Building in release mode (faster execution)..."
    cargo build -q --release --manifest-path=server/Cargo.toml
fi

echo ""
echo "==================================="
echo " SETTING UP PYTHON ENVIRONMENT"
echo "==================================="
if [ ! -d "$PROJECT_ROOT/.venv" ]; then
    echo "Creating virtual environment in .venv..."
    python3 -m venv "$PROJECT_ROOT/.venv"
fi

echo "Activating virtual environment..."
source "$PROJECT_ROOT/.venv/bin/activate"

echo "Installing dependencies..."
pip3 install -q -r "$PROJECT_ROOT/ai/requirements.txt"

echo ""
echo "==================================="
echo " STARTING ZAPPY AI TRAINING"
echo "==================================="
echo "Parameters:"
echo "  Timesteps: $TIMESTEPS"
echo "  Map Size:  ${WIDTH}x${HEIGHT}"
echo "  Frequency: $FREQ"
echo "  Team:      $TEAM"
echo "  Model:     $MODEL_NAME"
echo "  Load Model: ${LOAD_MODEL:-"None (train from scratch)"}"
echo "  Envs:      ${ENVS:-"Default (half of CPU cores)"}"
echo "==================================="
echo ""

# Ensure ai directory is in PYTHONPATH so python can resolve 'src' and 'training' modules
export PYTHONPATH="$PROJECT_ROOT/ai:$PYTHONPATH"

# Run the training script
ARGS=()
if [ -n "$ENVS" ]; then
    ARGS+=(--envs "$ENVS")
fi

python3 ai/training/training_env/train.py \
    --timesteps "$TIMESTEPS" \
    --width "$WIDTH" \
    --height "$HEIGHT" \
    --freq "$FREQ" \
    --team "$TEAM" \
    --model-name "$MODEL_NAME" \
    --load-model "$LOAD_MODEL" \
    "${ARGS[@]}"

echo ""
echo "==================================="
echo " Training sequence completed."
echo "==================================="
