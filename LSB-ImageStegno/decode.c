#include<stdio.h>
#include<string.h>
#include "types.h"
#include "common.h"
#include "decode.h"



status open_decode_files(DecodeInfo *decInfo)
{
	printf("opening required files\n");
	decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
	if(decInfo->fptr_stego_image == NULL)												// opening stego image
	{
		perror("fopen");
		fprintf(stderr, "Error:unable to open %s file\n", decInfo->stego_image_fname);
		return e_failure;
	}

	decInfo->fptr_secret = fopen( decInfo->secret_fname, "w" );
	if(decInfo->fptr_secret == NULL)
	{
		perror("fopen");
		fprintf(stderr, "Error:unable to open %s\n", decInfo->secret_fname);
		return e_failure;
	}
	return e_success;
}

status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
	decInfo->stego_image_fname = argv[2];
	if( argv[3] == NULL )
	{
		printf("output file name is not mentioned . so \"decode.txt\" is created by default\n");
		strcpy(decInfo->secret_fname , "decode.txt");
	 printf("%s\n",decInfo->secret_fname);
	}
	else
	{
		strcpy(decInfo->secret_fname , argv[3]);
	}

	if(open_decode_files(decInfo) == e_failure)
	{
		return e_failure;
	}
	else
	{
		printf("Done. file opened\n");
	}

	char bits[2];
	fread(bits, 1, 2, decInfo->fptr_stego_image);
	if(bits[0] == 0x42 && bits[1] == 0x4d)
	{
		printf("provide image file is \".bmp file\"\n");
	}
	else
	{
		printf("unknown type of file type is provided\n");
		return e_failure;
	}
	return e_success;
}
/*
int get_decode_file_size(FILE *fptr)
{
	fseek( fptr, 0, SEEK_END );
	return ftell(fptr);
}*/

status decode_lsb_to_int(int *size, char *data_buffer)
{
	for(int i=31 ; i >= 0 ; i--)
	{
		if( data_buffer[31 - i] & 1 )
		{
			*size = *size | 1 << i;
		}
	}
}

status decode_lsb_to_char(char *data, char *data_buffer)
{
	for(int i=7 ; i >= 0 ; i--)
	{
		if( data_buffer[7 - i] & 1)
		{	
			*data = *data | 1 << i;
		}
	}
}

status decode_magic_string(DecodeInfo *decInfo)
{
	int magic_size = 0;
	
	fseek(decInfo->fptr_stego_image , 54, SEEK_SET);
	fread(decInfo->data_buffer, 1, 32, decInfo->fptr_stego_image);
	decode_lsb_to_int(&magic_size, decInfo->data_buffer);
	
	if( magic_size < 1 )
	{
		printf("magic string not found\n");
		return e_failure;
	}
	
	char magic_string[magic_size+1];
	for(int  i = 0 ; i < magic_size ; i++)
	{
		magic_string[i] = 0;
		fread(decInfo->data_buffer, 1, 8, decInfo->fptr_stego_image);
		decode_lsb_to_char(&magic_string[i], decInfo->data_buffer);
	}
	magic_string[magic_size] = '\0';

	if( !(strcmp(magic_string, MAGIC_STRING)) )
	{
		printf("magic string matched\n");
		return e_success;
	}
	else
	{
		printf("magic string did not match\n");
		return e_failure;
	}
}

status decode_secret_file_extn(DecodeInfo *decInfo)
{
	int extn_size = 0;
	fread(decInfo->data_buffer, 1, 32, decInfo->fptr_stego_image);
	decode_lsb_to_int(&extn_size, decInfo->data_buffer);

	char file_extn[extn_size+1];
	for(int i=0 ; i < extn_size ; i++)
	{
		file_extn[i] = 0;
		fread(decInfo->data_buffer, 1, 8,decInfo->fptr_stego_image);
		decode_lsb_to_char(&file_extn[i], decInfo->data_buffer);
	}
	file_extn[extn_size] = '\0';

	char *output_file_extn = strtok(decInfo->secret_fname, ".");
	output_file_extn = strtok(NULL, ".");

	
	int output_file_extn_size = strlen(output_file_extn);

	if( !(strcmp(file_extn, output_file_extn)))
	{
		printf("extension matched\n");
	}
	else
	{
		printf("extension not matched. so change the extension\n");
	}

	strcat(decInfo->secret_fname, ".");
	strcat(decInfo->secret_fname, file_extn);

	strcpy(decInfo->secret_file_extn, file_extn);

	return e_success;
}

status decode_secret_file_size(DecodeInfo *decInfo)
{
	int secret_size = 0;
	fread(decInfo->data_buffer, 1, 32, decInfo->fptr_stego_image);
	decode_lsb_to_int(&secret_size, decInfo->data_buffer);
	decInfo->secret_file_size = secret_size;

	return e_success;
}

status decode_secret_file_data(DecodeInfo *decInfo)
{
	int data_size = decInfo->secret_file_size;
	char secret_file_data[data_size + 1];

	for(int i = 0; i < data_size ; i++)
	{
		secret_file_data[i] = 0;
		fread(decInfo->data_buffer, 1, 8, decInfo->fptr_stego_image);
		decode_lsb_to_char(&secret_file_data[i], decInfo->data_buffer);
	}

	secret_file_data[data_size] = '\0';
	rewind(decInfo->fptr_secret);
	fputs(secret_file_data, decInfo->fptr_secret);

	return e_success;
}

status do_decoding(DecodeInfo *decInfo)
{
	printf("---------------Decode procedure started----------------\n");

	printf("Decoding magic started\n");
	if( decode_magic_string(decInfo) == e_failure)
	{
		printf("Failed\n");
		return e_failure;
	}
	else
	{
		printf("Done\n");
	}
	
	printf("Decoding File extension.\n");
	if( decode_secret_file_extn(decInfo) == e_failure)
	{
		printf("Failed\n");
		return e_failure;
	}
	else
	{
		printf("Done\n");
	}

	printf("Decoding \"%s\" file size\n",decInfo->secret_fname);
	if( decode_secret_file_size(decInfo) == e_failure)
	{
		printf("Failed\n");
		return e_failure;
	}
	else
	{
		printf("Done\n");
	}

	printf("Decoding \"%s\" file data\n",decInfo->secret_fname);
	if( decode_secret_file_data(decInfo) == e_failure)
	{
		printf("Failed\n");
		return e_failure;
	}
	else
	{
		printf("Done\n");
	}

	printf("-----------------Decoding done successfully-----------------\n");

	return e_success;
}







