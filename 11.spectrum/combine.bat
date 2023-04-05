idf.py build

esptool.py  --chip esp32s3 merge_bin -o sp_test_0x0.bin --target-offset  0x0 --flash_mode dio --flash_freq 80m 0x0 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0x10000 build/hello-world.bin 
