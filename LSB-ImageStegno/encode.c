#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in case of RGB color image)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 * bpp is at offset 28, size is 2 bytes
 */

int get_image_size_for_bmp(EncodeInfo *encInfo)
{
    FILE *fptr_src_image = encInfo->fptr_src_image; // Copy to local var. Just to shorten the code
    uint width, height;
    unsigned short bpp;
    int img_size;

    // Seek to 18th byte
    fseek(fptr_src_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_src_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_src_image);
    printf("height = %u\n", height);

    // Get bits per pixel
    fseek(fptr_src_image, 28, SEEK_SET);
    fread(&bpp, sizeof(short), 1, fptr_src_image);
    printf("bpp = %hu\n", bpp);

    // Store values in encInfo
    encInfo->width = width;
    encInfo->height = height;
    encInfo->bpp = bpp;

    img_size = width * height * bpp; // Calculate image size
    return img_size;
}

/* Description: Open files
 * Inputs: Source image filename, Secret filename and Stego Image file name
 * Output: FILE pointers for above files
 * Return Value:
 *  n			e_success - If all files are opened,
 *  e_failure - On file errors
 */
status open_encode_files(EncodeInfo *encInfo)
{
	printf("opening required file\n");
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    //Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);
    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    //Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);
    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    //Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);
    	return e_failure;
    }

    // All files opened, return e_success
    return e_success;

}

OperationType check_operation_type(char *argv[])
{
	if( !strcmp(argv[1], "-e"))
		return e_encode;

	else if ( !strcmp(argv[1], "-d"))
		return e_decode;

	else
		return e_unsupported;
}

long get_file_size(FILE *fptr)
{
	fseek(fptr, 0,SEEK_END);
	return ftell(fptr);
}

status check_capacity( EncodeInfo *encInfo )
{
	printf("\nChecking files empty or not\n");
	int secret_file_size = get_file_size(encInfo->fptr_secret);
	encInfo->secret_file_size = secret_file_size;

	if( secret_file_size == 0 )
	{
		printf("Error: %s is empty.\n", encInfo->secret_fname);
	}
	else
		printf("Secret file is not empty\n");

	printf("Checking capacity of image file to handle secret file.\n");	

	int magic_string_size = strlen(MAGIC_STRING);

	char *secret_file_extn = strtok(encInfo->secret_fname,".");
	secret_file_extn = strtok(NULL, ".");
	int secret_file_extn_size = strlen(secret_file_extn);

	strcpy(encInfo->secret_file_extn, secret_file_extn);
	encInfo->secret_file_extn_size = secret_file_extn_size;
	int total_msg_size = sizeof(int) + magic_string_size + sizeof(int) + secret_file_extn_size + sizeof(int) + secret_file_size;

	int src_image_size = get_image_size_for_bmp(encInfo);

	if( src_image_size == 0 )
	{
		printf("%s is empty\n", encInfo->src_image_fname);
		return e_failure;
	}

	if( total_msg_size * 8 > src_image_size )
	{
		printf("cannot store secret message\n");
		return e_failure;
	}

	printf("Done. file size is upto requirement\n");
	return e_success;

}

status read_and_validate_file(char *argv[], EncodeInfo *encInfo)
{
	encInfo->src_image_fname = argv[2];
	encInfo->secret_fname = argv[3];
	
	if(argv[4] == NULL)
	{
		printf("output name not mention . creating \"stego_image.bmp\" ");
		encInfo-> stego_image_fname = "stego_image.bmp";
	}
	else
	{
		encInfo-> stego_image_fname = argv[4];
	}
	// open encode files
	
	open_encode_files(encInfo);

	char data[2];
	fread(data, 1, 2, encInfo->fptr_src_image);
	if( data[0] == 0x42 && data[1] == 0x4d )
	{
		printf("it is a .bmp file\n");
	}
	else
	{
		printf("it is not a .bmp file\n");
		return e_failure;
	}
/*
	if(check_capacity(encInfo) == e_failure);
	{
		return e_failure;
	}
	return e_success;
*/
	check_capacity(encInfo);
}

status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_stego_image)
{
	rewind(fptr_src_image);
	rewind(fptr_stego_image);

	char bmp_header[54];

	fread(bmp_header, 1, 54, fptr_src_image);
	fwrite(bmp_header, 1, 54, fptr_stego_image);
	
	return e_success;
}

status encode_byte_to_lsb(char data, char *image_buffer)
{
	for(int i=7;i>=0;i--)
	{
		if(data >> i & 1)
			image_buffer[7-i] = image_buffer[7-i] | 1;
		else
			image_buffer[7-i] = image_buffer[7-i] & ~1;
	}
}

status encode_int_to_lsb(int data, char *image_buffer)
{
	for(int i=31;i>=0;i--)
	{
		if( data >> i & 1 )
			image_buffer[31-i] = image_buffer[31-i] | 1;
		else
			image_buffer[31-i] = image_buffer[31-i] & ~1;
	}	
}

status encode_magic_string( const char *magic_string, EncodeInfo *encInfo )
{
	int magic_string_size = strlen(magic_string);
	
	fseek(encInfo->fptr_src_image, 54, SEEK_SET);
	fseek(encInfo->fptr_stego_image, 54, SEEK_SET);

	fread(encInfo->image_buffer, 1, 32, encInfo->fptr_src_image);
	encode_int_to_lsb(magic_string_size,encInfo->image_buffer);
	fwrite(encInfo->image_buffer, 1, 32, encInfo->fptr_stego_image);
	
	for(int i = 0; i < magic_string_size; i++)
	{	
	fread(encInfo->image_buffer, 1, 8, encInfo->fptr_src_image);
	encode_byte_to_lsb(magic_string[i], encInfo->image_buffer);
	fwrite(encInfo->image_buffer, 1, 8, encInfo->fptr_stego_image);
	}
	return e_success;
}

status encode_secret_file_extn( const char *file_extn, EncodeInfo *encInfo)
{
	fread(encInfo->image_buffer, 1, 32, encInfo->fptr_src_image);
	encode_int_to_lsb(encInfo->secret_file_extn_size, encInfo->image_buffer);
	fwrite(encInfo->image_buffer, 1, 32, encInfo->fptr_stego_image);
	
	for( int i = 0 ; i < strlen(file_extn) ; i++)
	{		
		fread(encInfo->image_buffer, 1, 8, encInfo->fptr_src_image);
		encode_byte_to_lsb(file_extn[i], encInfo->image_buffer);
		fwrite(encInfo->image_buffer, 1, 8, encInfo->fptr_stego_image);
	}
	return e_success;
}

status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
	fread(encInfo->image_buffer, 1, 32, encInfo->fptr_src_image);
	encode_int_to_lsb(file_size, encInfo->image_buffer);
	fwrite(encInfo->image_buffer, 1, 32, encInfo->fptr_stego_image);
	return e_success;
}

status encode_secret_file_data(EncodeInfo *encInfo)
{
	rewind(encInfo->fptr_secret);
	for( int i = 0; i < encInfo->secret_file_size ; i++ )
	{
		char data = fgetc(encInfo->fptr_secret);
		fread(encInfo->image_buffer, 1, 8, encInfo->fptr_src_image);
		encode_byte_to_lsb(data, encInfo->image_buffer);;
		fwrite(encInfo->image_buffer, 1, 8, encInfo->fptr_stego_image);
	}
	return e_success;
}

status copy_remaining_img_data( FILE *fptr_src, FILE *fptr_dest )
{
	char data;
	while( !(feof(fptr_src)))
	{
		data = fgetc(fptr_src);
		fputc(data,	fptr_dest);
	}
	return e_success;
}

status do_encoding(EncodeInfo *encInfo)
{
	printf("\n-----------------Encoding procedure started--------------------\n");
  	
	printf("copying magic string signature..\n");
	if(copy_bmp_header (encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
	{
		printf("Failed!!\n");
		e_failure;
	}	
	else
		printf("Done!!\n");

	printf("encoding magic string signature..\n");
	if( encode_magic_string(MAGIC_STRING, encInfo) == e_failure)
	{
		printf("Failed!!\n");
		return e_failure;
	}
	else	
		printf("DOne!!\n");
	
	printf("Encoding %s file extn\n", encInfo->secret_fname);
	if( encode_secret_file_extn(encInfo->secret_file_extn, encInfo) == e_failure)
	{
		printf("Failed!!\n");
		return e_failure;
	}
	else
		printf("Done..\n");

	printf("Encoding %s file size\n",encInfo->secret_fname);
	if( encode_secret_file_size(encInfo->secret_file_size, encInfo) == e_failure)
	{
		printf("Failed!!\n");
		return e_failure;
	}
	else
		printf("Done..\n");

	printf("Encoding %s file data\n", encInfo->secret_fname);
	if( encode_secret_file_data(encInfo) == e_failure)
	{
		printf("Failed!!\n");
		return e_failure;
	}
	else
		printf("Done..\n");

	printf("Copying left over data\n");
	if(copy_remaining_img_data( encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_failure)
	{
		printf("Failed!!\n");
		return e_failure;
	}
	else
		printf("Done..\n");

	printf("--------------------Encoding Done successfully--------------------\n");

	long img_size;
}
