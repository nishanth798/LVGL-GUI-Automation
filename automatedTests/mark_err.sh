#!/usr/bin/env bash
set -u

MODE=${1:-}
BIN="./build/simtest"
SUBDIR=""
GROUP_ARGS=()
SIMMODE=""

# --- START OF VALIDATION ---
if [ -z "$MODE" ]; then
  echo "ERROR: No variant selected!"
  echo "Usage: $0 [VARIANT] [OPTIONAL_TEST_GRP] [OPTIONAL_TEST_NAME]"
  echo "Available Options:"
  echo "  ALL     - Run every single test group"
  echo "  IN      - Run all Interactive (NKD-O) tests"
  echo "  GO      - Run all Go (NKD-GO) tests"
  echo "  RO      - Run all Read-Only (NKD-RO) tests"
  echo "  CMD     - Run Command tests"
  echo "  TO      - Run Timeout tests"
  echo "Available Specific Sub-Modes (Runs one specific group):"
  echo "  [Interactive]"
  echo "  IN_COM  - INT_Common_screen"
  echo "  IN_ACT  - INT_ActivationFlow"
  echo "  IN_MON  - INT_Monitoring_Screens"
  echo "  IN_ALE  - INT_Alerts_and_error_screens"
  echo "  IN_SET  - INT_ACT_Settings_Flow"
  echo ""
  echo "  [Go]"
  echo "  GO_COM  - GO_Common_screen"
  echo "  GO_ACT  - GO_ActivationFlow"
  echo "  GO_MON  - GO_MonitoringScreens"
  echo "  GO_ALE  - GO_Alerts_and_error_screens"
  echo "  GO_SET  - GO_ACT_Settings_Flow"
  echo ""
  echo "  [Read-Only]"
  echo "  RO_SCR  - RO_Screens"
  exit 1
fi
# --- END OF VALIDATION ---

case "$MODE" in
  ALL)
    SIMMODE="ALL"
    SUBDIR=""
    ;;
  IN)
    SIMMODE="IN"
    SUBDIR="NKD-O"
    ;;
  IN_COM)
    SIMMODE="IN"
    SUBDIR="NKD-O/NKD-COM"
    GROUP_ARGS=(-g INT_Common_screen)
    ;;
  IN_ACT)
    SIMMODE="IN"
    SUBDIR="NKD-O/NKD-ACT"
    GROUP_ARGS=(-g INT_ActivationFlow)
    ;;
  IN_MON)
    SIMMODE="IN"
    SUBDIR="NKD-O/NKD-MON"
    GROUP_ARGS=(-g INT_Monitoring_Screens)
    ;;
  IN_ALE)
    SIMMODE="IN"
    SUBDIR="NKD-O/NKD_ALE"
    GROUP_ARGS=(-g INT_Alerts_and_error_screens)
    ;;
  IN_SET)
    SIMMODE="IN"
    SUBDIR="NKD-O/NKD_SET"
    GROUP_ARGS=(-g INT_ACT_Settings_Flow)
    ;;
  GO)
    SIMMODE="GO"
    SUBDIR="NKD-GO"
    ;;
  GO_COM)
    SIMMODE="GO"
    SUBDIR="NKD-GO/NKD-COM"
    GROUP_ARGS=(-g GO_Common_screen)
    ;;
  GO_ACT)
    SIMMODE="GO"
    SUBDIR="NKD-GO/NKD-ACT"
    GROUP_ARGS=(-g GO_ActivationFlow)
    ;;
  GO_MON)
    SIMMODE="GO"
    SUBDIR="NKD-GO/NKD-MON"
    GROUP_ARGS=(-g GO_MonitoringScreens)
    ;;
  GO_ALE)
    SIMMODE="GO"
    SUBDIR="NKD-GO/NKD-ALE"
    GROUP_ARGS=(-g GO_Alerts_and_error_screens)
    ;;
  GO_SET)
    SIMMODE="GO"
    SUBDIR="NKD-GO/NKD_SET"
    GROUP_ARGS=(-g GO_ACT_Settings_Flow)
    ;;
  RO)
    SIMMODE="RO"
    SUBDIR="NKD-RO"
    ;;
  RO_SCR)
    SIMMODE="RO"
    SUBDIR="NKD-RO"
    GROUP_ARGS=(-g RO_Screens)
    ;;
  CMD)
    SIMMODE="CMD"
    SUBDIR="NKD-CMD"
    ;;
  TO)
    SIMMODE="TO"
    SUBDIR="NKD-TO"
    ;;
  *)
    echo "Unknown MODE: $MODE" >&2
    echo "Run '$0' without arguments to see valid options."
    exit 2
    ;;
esac

if [ -n "${MODE:-}" ]; then
  shift || true
fi

TEST_NAME=${1:-}
NAME_ARGS=()
if [ -n "${TEST_NAME:-}" ]; then
  NAME_ARGS=(-n "$TEST_NAME")
fi

BASE_REF_DIR="./ref_imgsNKD"
REF_DIR="${BASE_REF_DIR}/${SUBDIR}"
LOG="$(mktemp -t mark_err.XXXXXX.log)"
trap 'rm -f "$LOG"' EXIT

RADIUS=${RADIUS:-12}
THICK=${THICK:-4}

mkdir -p "$REF_DIR"

echo "Running tests in mode: $SIMMODE"
SIMTEST_MODE="$SIMMODE" "$BIN" -v "${GROUP_ARGS[@]}" "${NAME_ARGS[@]}" | tee "$LOG"
run_rc=${PIPESTATUS[0]}

ref=$(sed -nE 's/^[[:space:]]*-[[:space:]]*File:[[:space:]]*([^[:space:]]+).*/\1/p' "$LOG" | head -n1)
x=$(  sed -nE 's/^[[:space:]]*-[[:space:]]*At[[:space:]]*x:([0-9]+),[[:space:]]*y:[0-9]+.*/\1/p' "$LOG" | head -n1)
y=$(  sed -nE 's/^[[:space:]]*-[[:space:]]*At[[:space:]]*x:[0-9]+,[[:space:]]*y:([0-9]+).*/\1/p' "$LOG" | head -n1)

if [ -z "${ref:-}" ] || [ -z "${x:-}" ] || [ -z "${y:-}" ]; then
  exit "$run_rc"
fi

stem="$(basename "$ref" .png)"
ref_dir="$(dirname "$ref")"
mkdir -p "$ref_dir"
candidate1="${ref_dir}/${stem}_err.png"
candidate2="./${stem}_err.png"
in=""
if [ -f "$candidate1" ]; then
  in="$candidate1"
elif [ -f "$candidate2" ]; then
  mv "$candidate2" "$candidate1"
  in="$candidate1"
fi
out="${ref_dir}/${stem}_marked.png"
if [ -z "$in" ]; then
  exit "$run_rc"
fi
convert "$in" -stroke red -strokewidth "$THICK" -fill none -draw "circle $x,$y $((x+RADIUS)),$y" "$out" || exit 1
rm -f "$in"
exit "$run_rc"