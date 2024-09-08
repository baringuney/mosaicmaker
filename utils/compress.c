#include "../yesilbaglastirici.h"


// Function to calculate the average color in a 3x3 neighborhood
void calculate_average(unsigned char *image, unsigned char *new_image, int width, int height, int x, int y, int components, int compression) {
    int r_sum = 0, g_sum = 0, b_sum = 0;
    int count = 0;

    // Loop through the 3x3 block around the pixel (x, y)
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int nx = x + dx;
            int ny = y + dy;

            // Make sure the neighboring pixel is within bounds
            if (nx >= 0 && ny >= 0 && nx < width && ny < height) {
                unsigned char *pixel = &image[(ny * width + nx) * components];
                r_sum += pixel[0]; // Red
                g_sum += pixel[1]; // Green
                b_sum += pixel[2]; // Blue
                count++;
            }
        }
    }

    // Calculate the average color
    unsigned char avg_r = count > 0 ? r_sum / count : 0;
	unsigned char avg_g = count > 0 ? g_sum / count : 0;
	unsigned char avg_b = count > 0 ? b_sum / count : 0;

    // Set the pixel in the new image to the average color
    unsigned char *new_pixel = &new_image[((y * width)/compression + (x)/compression) * components];
    new_pixel[0] = avg_r; // Red
    new_pixel[1] = avg_g; // Green
    new_pixel[2] = avg_b; // Blue
}

int calculateCompression(int height, int width, int tolower)
{
	if(height * width > tolower * tolower)
	{
		int ret =  sqrt(height * width) / tolower;
		return ret + 1;
	}
	else
		return 1;
}

void process_image(const char *input_filename, const char *output_filename, int tolower) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr;
	int compression;
    // Open the input file
    FILE *infile = fopen(input_filename, "rb");
    if (!infile) {
        fprintf(stderr, "Cannot open input file %s\n", input_filename);
        return;
    }

    // Set up error handling and decompression structures
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);

    int width = cinfo.output_width;
    unsigned int height = cinfo.output_height;
    int components = cinfo.output_components;
    int row_stride = width * components;

	compression = calculateCompression(height, width , tolower);
    // Allocate memory for the entire image
    unsigned char *image = (unsigned char *)malloc(row_stride * height);
    unsigned char *new_image = (unsigned char *)malloc(((row_stride)) * (height)); // For storing the modified image

    // Read the entire image into memory
    while (cinfo.output_scanline < height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = image + (cinfo.output_scanline * row_stride);
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    // Process each pixel in the image
    for (unsigned int y = 0; y < height; y+=compression) {
        for (int x = 0; x < width; x+=compression) {
            calculate_average(image, new_image, width, height, x, y, components, compression);
        }
    }

    // Open the output file
    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        fprintf(stderr, "Cannot open output file %s\n", output_filename);
        free(image);
        free(new_image);
        jpeg_destroy_decompress(&cinfo);
        return;
    }

    // Set up compression structures
    cinfo_out.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, outfile);

    // Copy basic image information to the output structure
    cinfo_out.image_width = width/compression;
    cinfo_out.image_height = height/compression;
    cinfo_out.input_components = components;
    cinfo_out.in_color_space = cinfo.out_color_space;

    jpeg_set_defaults(&cinfo_out);
    jpeg_start_compress(&cinfo_out, TRUE);

    // Write the processed image to the output file
    while (cinfo_out.next_scanline < height/compression) {
        unsigned char *buffer_array[1];
        buffer_array[0] = new_image + (cinfo_out.next_scanline * row_stride);
        jpeg_write_scanlines(&cinfo_out, buffer_array, 1);
    }

    // Finish compression and clean up
    jpeg_finish_compress(&cinfo_out);
    jpeg_finish_decompress(&cinfo);

    jpeg_destroy_compress(&cinfo_out);
    jpeg_destroy_decompress(&cinfo);

    free(image);
    free(new_image);

    fclose(infile);
    fclose(outfile);

    printf("Image processed and saved as %s\n", output_filename);
}