#include "../yesilbaglastirici.h"

unsigned char getColor(unsigned char rgb, int* range) {
    // Clamp the range values to [0, 255]
    if (range[0] < 0) range[0] = 0;
    if (range[1] > 255) range[1] = 255;

    // Calculate the difference and ratio
    int range_diff = range[1] - range[0];  // Use int for calculation

    // Calculate the result without floating-point arithmetic
    unsigned char result = (unsigned char)((range_diff * rgb) / 256 + range[0]);
    return result;
}

void filter_to_pink(const char *input_filename, const char *output_filename) {
    // Decompression structures
    struct jpeg_decompress_struct cinfo;
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr;

    // Open the input file
    FILE *infile = fopen(input_filename, "rb");
    if (!infile) {
        fprintf(stderr, "Cannot open input file %s\n", input_filename);
        return;
    }

    // Set up error handling
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);

    // Read the header
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    // Prepare for image processing
    int row_stride = cinfo.output_width * cinfo.output_components;  // Number of bytes in a row
    unsigned char *buffer = (unsigned char *)malloc(row_stride);

    // Open the output file
    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        fprintf(stderr, "Cannot open output file %s\n", output_filename);
        jpeg_destroy_decompress(&cinfo);
        fclose(infile);
        free(buffer);
        return;
    }

    // Set up output compression structure
    cinfo_out.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, outfile);

    // Copy basic image information to output structure
    cinfo_out.image_width = cinfo.output_width;
    cinfo_out.image_height = cinfo.output_height;
    cinfo_out.input_components = cinfo.output_components;
    cinfo_out.in_color_space = cinfo.out_color_space;

    jpeg_set_defaults(&cinfo_out);
    jpeg_start_compress(&cinfo_out, TRUE);

    // Process each scanline
    while (cinfo.output_scanline < cinfo.output_height) {
        jpeg_read_scanlines(&cinfo, &buffer, 1);

        // Apply the blue filter: Zero out red and green channels
        for (int i = 0; i < row_stride; i += cinfo.output_components) {
            // Assuming the image is in RGB format (3 components per pixel)
            buffer[i] = getColor(buffer[i],(int[]){150, 255});    // Red component
            buffer[i + 1] = getColor(buffer[i+1],(int[]){50, 100});  // Green component
            buffer[i + 2] = getColor(buffer[i+2],(int[]){100, 255});
        }

        jpeg_write_scanlines(&cinfo_out, &buffer, 1);
    }

    // Finish decompression and compression
    jpeg_finish_decompress(&cinfo);
    jpeg_finish_compress(&cinfo_out);

    // Release memory
    jpeg_destroy_decompress(&cinfo);
    jpeg_destroy_compress(&cinfo_out);
    free(buffer);

    // Close files
    fclose(infile);
    fclose(outfile);

    printf("Blue filter applied to %s and saved as %s\n", input_filename, output_filename);
}

