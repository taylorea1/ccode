#!/bin/bash

for channel in $(seq -w 0 31)
do
    OUTPUT_DIR="channel_${channel}"

    mkdir -p "$OUTPUT_DIR"

    find . -maxdepth 1 -type f \
    \( -name "*channel_${channel}_*.pdf" \
    -o -name "*channel_${channel}_*.png" \
    -o -name "*channel_${channel}_*.csv" \
    -o -name "*channel_${channel}_*.root" \) \
    -exec mv {} "$OUTPUT_DIR"/ \;

    echo "Finished channel ${channel}"
done

echo
echo "All channels completed."