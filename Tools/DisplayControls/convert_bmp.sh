#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 continuation_in_active.bmp -fv -nc
./Bmp2Ssd1306 continuation_in_inactive.bmp -fv -nc
./Bmp2Ssd1306 continuation_out_active.bmp -fv -nc
./Bmp2Ssd1306 continuation_out_inactive.bmp -fv -nc

echo "All done! You can now run:"

