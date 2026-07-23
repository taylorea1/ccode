#!/bin/bash

# rm -f adc_tdc_histogram*.pdf
# rm -f tdc_adc_blocks*.pdf
read -p "Enter ADC threshold: " THRESHOLD

for row in {0..7}; do
    for col in {0..3}; do
        echo "Running: row=$row col=$col threshold=$THRESHOLD"
        root -l -b -q "thresholdadc_tdc.C($row,$col,$THRESHOLD)"
    done
done
echo "Combining PDFs..."
pdfunite adc_tdc_histogram*.pdf tdc_adc_blocks*.pdf combined_threshold_scan.pdf
echo "Done: combined_threshold_scan.pdf"


