set(COMPONENT_SRCS 
    "app_main.c"
    "page/page_fft.c"
    "fft_soft/fft.c"
    )
set(COMPONENT_ADD_INCLUDEDIRS  "." "page" "fft_soft")


register_component()
