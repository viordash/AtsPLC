#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 disable_cursor_0.bmp -fv -nc
./Bmp2Ssd1306 disable_cursor_1.bmp -fv -nc

echo "All done! You can now run:"

