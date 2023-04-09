if(ESP_PLATFORM)

file(GLOB_RECURSE SOURCES lv_examples/*.c)
message("===================================Hello, world!===================================")
idf_component_register(SRCS ${SOURCES}
                       INCLUDE_DIRS .
                       REQUIRES lvgl)

endif()
