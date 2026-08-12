#!/bin/bash

# OUTPUT_DIR="tdc_histograms"

# mkdir -p "$OUTPUT_DIR"

# find . -maxdepth 1 -type f \
# \( -iname "*tdc*.pdf" \
# -o -iname "*tdc*.png" \
# -o -iname "*tdc*.root" \
# -o -iname "*tdc*.csv" \) \
# -exec mv {} "$OUTPUT_DIR"/ \;

# echo "Finished."
# echo "TDC files moved to $OUTPUT_DIR"

#!/bin/bash

# OUTPUT_DIR="active_analyzer_files"

# mkdir -p "$OUTPUT_DIR"

# find . -maxdepth 1 -type f \
# \( -iname "*activeAna*" \
# -o -iname "*active_analyzer*" \) \
# -exec mv {} "$OUTPUT_DIR"/ \;

# echo "Finished."
# echo "Files moved to $OUTPUT_DIR"


#!/bin/bash

OUTPUT_DIR="ROOT_macros"

mkdir -p "$OUTPUT_DIR"

find . -maxdepth 1 -type f -name "*.C" \
-exec mv {} "$OUTPUT_DIR"/ \;

echo "Finished."
echo "All .C files moved to $OUTPUT_DIR"