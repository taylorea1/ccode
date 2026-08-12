#!/usr/bin/env bash

set -euo pipefail
shopt -s nullglob

# Run from the directory containing this script, the ROOT macro, and its output.
SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

MACRO="eventracker_corrected.C"

if [[ ! -f "$MACRO" ]]; then
    echo "Error: $MACRO was not found in:"
    echo "  $SCRIPT_DIR"
    echo "Put this script and $MACRO in the same directory."
    exit 1
fi

if ! command -v root >/dev/null 2>&1; then
    echo "Error: ROOT is not available in this terminal."
    exit 1
fi

read -r -p "First channel [0]: " FIRST_CHANNEL
FIRST_CHANNEL="${FIRST_CHANNEL:-0}"

read -r -p "Last channel [31]: " LAST_CHANNEL
LAST_CHANNEL="${LAST_CHANNEL:-31}"

if ! [[ "$FIRST_CHANNEL" =~ ^[0-9]+$ &&
        "$LAST_CHANNEL" =~ ^[0-9]+$ &&
        "$FIRST_CHANNEL" -ge 0 &&
        "$LAST_CHANNEL" -le 31 &&
        "$FIRST_CHANNEL" -le "$LAST_CHANNEL" ]]; then
    echo "Error: enter a valid channel range from 0 through 31."
    exit 1
fi

for channel in $(seq "$FIRST_CHANNEL" "$LAST_CHANNEL"); do
    channel_padded="$(printf '%02d' "$channel")"
    pid_pdf="channel_${channel_padded}_PID.pdf"
    final_pdf="channel_${channel_padded}_events_and_PID.pdf"

    echo
    echo "============================================================"
    echo "Running event tracker for channel $channel"
    echo "============================================================"

    # Remove only this channel's old generated documents.
    rm -f -- "$pid_pdf" "$final_pdf"

    root -l -b -q "${MACRO}++g(${channel})"

    if [[ -f "$final_pdf" ]]; then
        echo "Created: $final_pdf"
    else
        echo "Warning: $final_pdf was not created."
    fi
done

echo
echo "Finished."
echo "Each processed channel has one document:"
echo "  channel_XX_events_and_PID.pdf"