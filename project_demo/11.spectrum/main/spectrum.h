#ifndef APP_SPECTRUM_H
#define APP_SPECTRUM_H
#include "spectrum_gui.h"
extern struct APP_OBJ spectrum_app;
void generate_spectrum(float *fft_buf, int32_t len);

#endif