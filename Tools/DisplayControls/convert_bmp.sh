#!/usr/bin/env bash

set -e
set -u

./Bmp2Ssd1306 cmp_equal_active.bmp
./Bmp2Ssd1306 cmp_equal_inactive.bmp
./Bmp2Ssd1306 cmp_greate_inactive.bmp
./Bmp2Ssd1306 cmp_greatel_active.bmp
./Bmp2Ssd1306 cmp_greate_or_equal_active.bmp
./Bmp2Ssd1306 cmp_greate_or_equal_inactive.bmp
./Bmp2Ssd1306 cmp_less_active.bmp
./Bmp2Ssd1306 cmp_less_inactive.bmp
./Bmp2Ssd1306 cmp_less_or_equal_active.bmp
./Bmp2Ssd1306 cmp_less_or_equal_inactive.bmp
./Bmp2Ssd1306 dec_output_active.bmp
./Bmp2Ssd1306 dec_output_inactive.bmp
./Bmp2Ssd1306 direct_output_active.bmp
./Bmp2Ssd1306 direct_output_inactive.bmp
./Bmp2Ssd1306 inc_output_active.bmp
./Bmp2Ssd1306 inc_output_inactive.bmp
./Bmp2Ssd1306 input_close_active.bmp
./Bmp2Ssd1306 input_close_inactive.bmp
./Bmp2Ssd1306 input_open_active.bmp
./Bmp2Ssd1306 input_open_inactive.bmp
./Bmp2Ssd1306 reset_output_active.bmp
./Bmp2Ssd1306 reset_output_inactive.bmp
./Bmp2Ssd1306 set_output_active.bmp
./Bmp2Ssd1306 set_output_inactive.bmp
./Bmp2Ssd1306 timer_msec_active.bmp
./Bmp2Ssd1306 timer_msec_inactive.bmp
./Bmp2Ssd1306 timer_sec_active.bmp
./Bmp2Ssd1306 timer_sec_inactive.bmp

echo "All done! You can now run:"

