--chip esp32c5 ^
-b 460800 ^
--before default-reset ^
--after hard-reset ^
write-flash ^
--flash-mode dio ^
--flash-size 2MB ^
--flash-freq 80m ^
0x2000 build/bootloader/bootloader.bin ^
0x8000 build/partition_table/partition-table.bin ^
0x10000 build/udp_server.bin