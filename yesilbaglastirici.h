#ifndef YESILBAGLASTIRICI_H
#define YESILBAGLASTIRICI_H

#include <stdio.h>
#include <jpeglib.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void process_image(const char *input_filename, const char *output_filename, int tolower);
void filter_to_pink(const char *input_filename, const char *output_filename);
unsigned char getColor(unsigned char rgb, int* range);

#endif
