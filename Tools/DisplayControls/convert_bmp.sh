#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 cmp_not_equal_active.bmp -fv -nc
./Bmp2Ssd1306 cmp_not_equal_inactive.bmp -fv -nc

echo "All done! You can now run:"

