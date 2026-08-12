# #!/bin/bash

# mkdir -p adc/tdc_histograms

# mv adc_tdc_histogram* adc/tdc_histograms


#!/bin/bash

OUTPUT_DIR="adctdchisto_scatter_blocks"

mkdir -p "$OUTPUT_DIR"

find . -maxdepth 1 -type f \
\( -name "*adc_tdc_histogram*.png" \
-o -name "*scatter_" \
-o -name "*adc_tdc_histogram*.root" \
-o -name "*adc_tdc_histogram*.csv" \) \
-exec mv {} "$OUTPUT_DIR"/ \;

echo "Finished."
echo "Files moved to $OUTPUT_DIR"