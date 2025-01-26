#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 wif_sta_active.bmp -fv -nc
./Bmp2Ssd1306 wif_sta_error.bmp -fv -nc
./Bmp2Ssd1306 wif_sta_inactive.bmp -fv -nc

echo "All done! You can now run:"

