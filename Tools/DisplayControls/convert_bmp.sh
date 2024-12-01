#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 wifi_binding_active.bmp -fv -nc
./Bmp2Ssd1306 wifi_binding_inactive.bmp -fv -nc

echo "All done! You can now run:"

