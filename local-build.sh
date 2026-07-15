docker run -t \
-e IDF_TARGET="esp32c5" \
-v "$PWD:/app/project" \
-w "/app/project" \
espressif/idf:latest /bin/bash -c 'git config --global --add safe.directory "*" &&  idf.py build'
# idf.py  build
# idf.py set-target esp32c5 &&
# idf.py fullclean && 
# idf.py menuconfig && 
# &&  idf.py add-dependency "espressif/esp_csi_gain_ctrl^0.1.5"
# sudo rm -rf build
# idf.py add-dependency "espressif/esp_csi_gain_ctrl^0.1.5"
# idf.py save-defconfig &&