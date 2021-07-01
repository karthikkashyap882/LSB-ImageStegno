/*
Name : Karthik s kashyap
Project name: LSB steganography
Date: 10-03-2021
*/


#include <stdio.h>
#include <string.h>
#include "encode.h"
#include "decode.h"
#include "encode.c"
#include "decode.c"

// Do not use this code initially.
// Implement this function only after individually testing encode and decode part

int main(int argc, char *argv[])
{
    //Parse cmdline arguments
	EncodeInfo encInfo;
	DecodeInfo decInfo;

    int type = check_operation_type(argv);
	
	if(type == e_encode)
	{
		if( argc < 4 )
		{
			printf("command line arguements are not proper\n");
			return 1;
		}
		if( read_and_validate_file(argv, &encInfo) == e_failure)
		{
			return 1;
		}
		if( do_encoding(&encInfo) == e_failure)
		{
			printf("Error in opening file\n" );
			return 1;
		}
	}
	else if( type  == e_decode)
	{
		if( argc < 3 )
		{
			printf("command line arguements are not proper\n");
			return 1;
		}
		if( read_and_validate_decode_args(argv, &decInfo) == e_failure)
			return 1;

		do_decoding(&decInfo);
	}

    return 0;
}
