#ifndef ENCODE_H
#define ENCODE_H

#include "types.h" // Contains user defined types
#include "common.h"

/* Structure to store information required to
 * encode secret file to source image.
 */


typedef struct _EncodeInfo  
{
    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char secret_file_extn[MAX_SECRET_FILE_EXTN];
    int secret_file_size;
	int secret_file_extn_size;

    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint width, height;
    uint bpp; //bits per pixel
	char image_buffer[30];

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

} EncodeInfo;


/* Encode function prototype */

/* Read and validate args from argv */
OperationType read_and_validate_cmdline_args(char *argv[], EncodeInfo *encInfo);
/* Perform the encoding */
status do_encoding(EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
status open_encode_files(EncodeInfo *encInfo);

/* check capacity */
status check_capacity(EncodeInfo *encInfo);

/* Get image size */
int get_image_size_for_bmp(EncodeInfo *encInfo);

/* Get file size */
long get_file_size(FILE *fptr);

/* Copy bmp image header */
status copy_bmp_header(FILE * fptr_src_image, FILE *fptr_stego_image);

/* Store Magic String */
status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Encode secret file extenstion */
status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode secret file size */
status encode_secret_file_size(int file_size, EncodeInfo *encInfo);

/* Encode secret file data*/
status encode_secret_file_data(EncodeInfo *encInfo);

/* Encode function, which does the real encoding */
status encode_data(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image);

/* Encode a byte into LSB of image data array */
status encode_byte_to_lsb(char data, char *image_buffer);

/* Copy remaining image bytes from src to stego image after encoding */
status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif
