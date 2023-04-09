# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/crisqifawei/esp/esp-idf/components/bootloader/subproject"
  "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader"
  "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix"
  "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix/tmp"
  "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix/src/bootloader-stamp"
  "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix/src"
  "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/crisqifawei/esp/ESP32S3-SP/22.uac_fft/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
