#include "yesilbaglastirici.h"

#include <stdio.h>
#include <string.h>
#define CONTRAST 60

// Function to check if a file has a .jpg or .jpeg extension
int is_jpeg_file(const char *filename) {
    const char *dot = strrchr(filename, '.');  // Find the last occurrence of '.'
    if (!dot || dot == filename) {
        return 0;  // No extension found, or it's the first character (invalid)
    }
    // Check if the extension is .jpg or .jpeg
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) {
        return 1;  // It's a JPEG file
    }
    return 0;  // Not a JPEG file
}

#include "yesilbaglastirici.h"

// Function to calculate the average color in a 3x3 neighborhood
void putimage(const char *input_filename, const char *material_filename, const char *output_filename) {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_decompress_struct cmat;
    struct jpeg_compress_struct cinfo_out;
    struct jpeg_error_mgr jerr;

    // Open the input file
    FILE *infile = fopen(input_filename, "rb");
    if (!infile) {
        fprintf(stderr, "Cannot open input file %s\n", input_filename);
        return;
    }

    // Open the material image file
    FILE *matfile = fopen(material_filename, "rb");
    if (!matfile) {
        fprintf(stderr, "Cannot open input file %s\n", material_filename);
        fclose(infile);
        return;
    }

    // Set up error handling and decompression structures
    cinfo.err = jpeg_std_error(&jerr);
    cmat.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_create_decompress(&cmat);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_stdio_src(&cmat, matfile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_read_header(&cmat, TRUE);
    jpeg_start_decompress(&cinfo);
    jpeg_start_decompress(&cmat);

    int width = cinfo.output_width;
    unsigned int height = cinfo.output_height;
    int components = cinfo.output_components;
    int row_stride = width * components;

    int mat_width = cmat.output_width;
    unsigned int mat_height = cmat.output_height;
    int mat_components = cmat.output_components;
    int mat_row_stride = mat_width * mat_components;

	int new_width = mat_width * width;
    unsigned int new_height = mat_height * height;
    int new_components = cmat.output_components;
    int new_row_stride = new_width * new_components;

    // Allocate memory for the entire input image and material image
	unsigned char *image = (unsigned char *)malloc(row_stride * height);
	if (image == NULL) {
    	fprintf(stderr, "Memory allocation for image failed.\n");
    	exit(1);
	}

	unsigned char *mat_image = (unsigned char *)malloc(mat_row_stride * mat_height);
	if (mat_image == NULL) {
    	fprintf(stderr, "Memory allocation for mat_image failed.\n");
    	free(image);
    	exit(1);
	}

	unsigned char *new_image = (unsigned char *)malloc(new_row_stride * new_height);
	if (new_image == NULL) {
 	   fprintf(stderr, "Memory allocation for new_image failed files are probably too big.\n");
  	   free(image);
  	   free(mat_image);
   	   exit(1);
	}

    // Read the entire input image into memory
    while (cinfo.output_scanline < height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = image + (cinfo.output_scanline * row_stride);
        jpeg_read_scanlines(&cinfo, buffer_array, 1);
    }

    // Read the entire material image into memory
    while (cmat.output_scanline < mat_height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = mat_image + (cmat.output_scanline * mat_row_stride);
        jpeg_read_scanlines(&cmat, buffer_array, 1);
    }

    // Now we copy the material image to every pixel in the input image
    for (unsigned int y = 0; y < new_height; y++) {
    for (int x = 0; x < new_width; x++) {
        int new_pixel_index = (y * new_width + x) * new_components;  // Use new_width here

        int mat_x = x % mat_width;
		int mat_y = y % mat_height;
        int mat_pixel_index = (mat_y * mat_width + mat_x) * mat_components;

		int img_x = x / mat_width;
		int img_y = y / mat_height;
		int img_pixel_index = (img_y * width + img_x) * components;
        // Copy material image pixel to the new image
        new_image[new_pixel_index] = getColor(mat_image[mat_pixel_index],(int[]){image[img_pixel_index] - CONTRAST, image[img_pixel_index] + CONTRAST});         // Red
        new_image[new_pixel_index + 1] = getColor(mat_image[mat_pixel_index + 1],(int[]){image[img_pixel_index + 1] - CONTRAST, image[img_pixel_index + 1] + CONTRAST}); // Green
        new_image[new_pixel_index + 2] = getColor(mat_image[mat_pixel_index + 2],(int[]){image[img_pixel_index + 2] - CONTRAST, image[img_pixel_index + 2] + CONTRAST}); // Blue
	}
}

    // Open the output file
    FILE *outfile = fopen(output_filename, "wb");
    if (!outfile) {
        fprintf(stderr, "Cannot open output file %s\n", output_filename);
        free(image);
        free(mat_image);
        free(new_image);
        jpeg_destroy_decompress(&cinfo);
        jpeg_destroy_decompress(&cmat);
        return;
    }

    // Set up compression structures
    cinfo_out.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo_out);
    jpeg_stdio_dest(&cinfo_out, outfile);

    // Copy basic image information to the output structure
    cinfo_out.image_width = new_width;
    cinfo_out.image_height = new_height;
    cinfo_out.input_components = components;
    cinfo_out.in_color_space = cinfo.out_color_space;

    jpeg_set_defaults(&cinfo_out);
    jpeg_start_compress(&cinfo_out, TRUE);

    // Write the processed image to the output file
    while (cinfo_out.next_scanline < cinfo_out.image_height) {
        unsigned char *buffer_array[1];
        buffer_array[0] = new_image + (cinfo_out.next_scanline * new_row_stride);
        jpeg_write_scanlines(&cinfo_out, buffer_array, 1);
    }

    // Finish compression and clean up
    jpeg_finish_compress(&cinfo_out);
    jpeg_finish_decompress(&cinfo);
    jpeg_finish_decompress(&cmat);

    jpeg_destroy_compress(&cinfo_out);
    jpeg_destroy_decompress(&cinfo);
    jpeg_destroy_decompress(&cmat);

    free(image);
    free(new_image);
    free(mat_image);

    fclose(infile);
    fclose(outfile);
    fclose(matfile);

    printf("Image processed and saved as %s\n", output_filename);
}



int main(int argc, char *argv[]) {
    // Check if exactly two arguments are passed (excluding program name)
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input.jpg> <output.jpg>\n", argv[0]);
        return 1;  // Return error code
    }

    // Check if the first argument is a JPEG file
    if (!is_jpeg_file(argv[1])) {
        fprintf(stderr, "Error: %s is not a valid .jpg or .jpeg file.\n", argv[1]);
        return 1;
    }

    // Check if the second argument is a JPEG file
    if (!is_jpeg_file(argv[2])) {
        fprintf(stderr, "Error: %s is not a valid .jpg or .jpeg file.\n", argv[2]);
        return 1;
    }

    // Check if the third argument is a JPEG file
    if (!is_jpeg_file(argv[3])) {
        fprintf(stderr, "Error: %s is not a valid .jpg or .jpeg file.\n", argv[2]);
        return 1;
    }

    // If both checks pass, proceed with the rest of the program
	process_image(argv[1],"/images/input.jpg", 150);
	process_image(argv[2],"/images/mat.jpg", 60);
    putimage("images/input.jpg","images/mat.jpg",argv[3]);

    // Proceed with your JPEG processing here

    return 0;  // Success
}
