#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 square_wave_generator_active.bmp -fv -nc
./Bmp2Ssd1306 square_wave_generator_inactive.bmp -fv -nc

echo "All done! You can now run:"

