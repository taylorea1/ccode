#!/bin/bash

OUTPUT_DIR="adc_histograms"

mkdir -p "$OUTPUT_DIR"

find . -maxdepth 1 -type f \
\( -iname "*adc*.pdf" \
-o -iname "*adc*.png" \
-o -iname "*adc*.root" \
-o -iname "*adc*.csv" \) \
-exec mv {} "$OUTPUT_DIR"/ \;

echo "Finished."
echo "ADC files moved to $OUTPUT_DIR"