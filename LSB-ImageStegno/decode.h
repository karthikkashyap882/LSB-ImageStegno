#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types
#include "common.h"

/* Structure to store information required to
 * decode secret file from stego image.
 */

typedef struct _DecodeInfo
{
    /* Retrieved Secret File Info */
    char secret_fname[255];
    FILE *fptr_secret;
    char secret_file_extn[MAX_FILE_SUFFIX];
    int secret_file_size;
	char data_buffer[MAX_IMAGE_BUF_SIZE];

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;                 //bits per pixel

} DecodeInfo;


/* Encode function prototype */

/* Perform the decoding */
status do_decoding(DecodeInfo *decInfo);

status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Get File pointers for i/p and o/p files */
status open_decode_files(DecodeInfo *decInfo);

/* Get file size */
int get_decode_file_size(FILE *fptr);

/* Store Magic String */
status decode_magic_string(DecodeInfo *decInfo);

/* Encode secret file extenstion */
status decode_secret_file_extn(DecodeInfo *decInfo);

/* Encode secret file size */
status decode_secret_file_size(DecodeInfo *decInfo);

/* Encode secret file data*/
status decode_secret_file_data(DecodeInfo *decInfo);

/* Encode a byte into LSB of image data array */
status decode_lsb_to_char(char *data, char *data_buffer);

status decode_lsb_to_int(int *size, char *data_buffer);

#endif
