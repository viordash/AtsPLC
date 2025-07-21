#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 copy_cursor_0.bmp -fv -nc
./Bmp2Ssd1306 copy_cursor_1.bmp -fv -nc
./Bmp2Ssd1306 delete_cursor_0.bmp -fv -nc
./Bmp2Ssd1306 delete_cursor_1.bmp -fv -nc
./Bmp2Ssd1306 moving_up_down_0.bmp -fv -nc
./Bmp2Ssd1306 moving_up_down_1.bmp -fv -nc

echo "All done! You can now run:"

