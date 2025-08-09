#include <stdio.h>
#include<string.h>
#include "types.h"
#include "common.h"
#include "decode.h"


Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Check if the stego image file has a .bmp extension
    if (strstr(argv[2], ".bmp") != NULL)
    {
        decInfo->stego_image_fname = argv[2];// Assign stego image filename
    }
    else
    {
        printf("error: stego image must be a .bmp file\n");// Invalid stego file type
        return e_failure;
    }

    // Check if output file name is provided
    if (argv[3] != NULL)
    {
        // Check if output file has a .txt extension
        if (strstr(argv[3], ".txt") != NULL)
        {
            strcpy(decInfo->output_image_fname, argv[3]);// Assign output image file name
        }
        else
        {
            printf("error: Output file is not .txt\n");// Not .txt, use default name
            strcpy(decInfo->output_image_fname, "output");// Manually set output image file name  as output
        }
    }
    else
    {
        strcpy(decInfo->output_image_fname, "output");//use default name
    }

    return e_success;
}
Status open_file(DecodeInfo *decInfo)
{
    // Stego Image file
    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "r");
    // Do Error handling
    if (decInfo->fptr_stego_image ==NULL)

    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->stego_image_fname);

    	return e_failure;
    }
    // No failure return e_success
    return e_success;
}
//skip bmp image header 
Status skip_bmp_header(FILE *fptr_stego_image) 
{
    //skip the 54bytes from stego_image
    if (fseek(fptr_stego_image, 54, SEEK_SET) != 0) 
    {
        return e_failure;
    }
    return e_success;
}
//decode byte from lsb
Status decode_byte_from_lsb(char *image_buffer, char *data)
{
    char decode_byte = 0;

    for (int i = 0; i < 8; i++)
    {
        // Get LSB from image_buffer[i]
        char lsb = image_buffer[i] & 1;

        // Set the getting bit in decode_byte
        decode_byte |= (lsb << (7-i));
    }

    *data = decode_byte; // Store the result in data
    return e_success;
}
//decode data from image
Status decode_data_from_image(FILE *fptr_stego_image, char *data, int size)
{
    char image_block[8];// Buffer to hold 8 bytes of image data
    for (int i = 0; i < size; i++)// Loop through each byte to be decoded
    {
        if (fread(image_block, 8, 1, fptr_stego_image) !=1) // Read 8 bytes from the stego image
        {
            //if not means print error msg
            printf("error: Unable to read image data for decoding\n");
            return e_failure;
        }
        
        // Decode 1 byte of hidden data from the 8 LSBs of image_block
        decode_byte_from_lsb(image_block, &data[i]);
    }
    return e_success;// Return success after decoding all bytes
}
//decode  Magic String 
Status decode_magic_string(FILE *fptr_stego_image)
{   
    char magic_string[strlen(MAGIC_STRING) + 1];// Allocate buffer to store the decoded magic string(1 for null terminator)
    if (decode_data_from_image(fptr_stego_image, magic_string, strlen(MAGIC_STRING)) != e_success)
    {
        //decode the data from the image if not means print error msg
        printf("ERROR: not to decode magic string\n");
        return e_failure;
    }
    magic_string[strlen(MAGIC_STRING)] = '\0';//terminate the null from the magic string
    if (strcmp(magic_string, MAGIC_STRING) != 0)// Compare decoded string with expected magic string if not means print error msg
    {
        return e_failure;
    }
    return e_success;
}
//decode file extension size
Status decode_file_extn_size(DecodeInfo *decInfo, unsigned int *extn_size) 
{
    unsigned int decoded_size = 0;
    unsigned char lsb_block[8];  // Buffer to hold 8 bytes from stego image
    char size_byte;              // Holds 1 decoded byte from LSBs

    for (int i = 0; i < 4; i++)  // Loop 4 times to get 4 bytes = 32 bits = 1 unsigned int
    {
    // Read 8 bytes from the stego image 
    fread(lsb_block, 8, 1, decInfo->fptr_stego_image);

    // Decode 1 byte (8 bits) from the 8 LSBs
    decode_byte_from_lsb((char *)lsb_block, &size_byte);

    // Left shift previous bits by 8 and OR with the new byte
    decoded_size = (decoded_size << 8) | (unsigned char)size_byte;
}

// Finally, store the result in the output variable
*extn_size = decoded_size;
return e_success;
}
//decode file extension
Status decode_file_extn(DecodeInfo *decInfo, unsigned int extn_size)
{
    // decode extension into file_extn
    if (decode_data_from_image(decInfo->fptr_stego_image, decInfo->file_extn, extn_size) != e_success)
    {
        //if not means print error msg
        printf("ERROR: Failed to decode file extension\n");
        return e_failure;
    }

    decInfo->file_extn[extn_size] = '\0';  // Null terminate
    return e_success;
}
//open the decode files
Status open_decode_files(DecodeInfo *decInfo) 
{
    // Concatenate the decoded extension to output filename
    strcat(decInfo->output_image_fname, decInfo->file_extn);
    // Open the file in write binary mode
    decInfo->fptr_output_image = fopen(decInfo->output_image_fname, "w");
    //check the file is opened or not ,if not means print error msg
    if (decInfo->fptr_output_image == NULL)
    {
        perror("fopen");
    	fprintf(stderr, "error: Unable to open file %s\n", decInfo->output_image_fname);

    	return e_failure;
    }
    return e_success;
}
//decode file size
Status decode_file_size(DecodeInfo *decInfo, unsigned int *decoded_size)
{
    unsigned int size = 0;// Initialize variable to store the decoded size
    char decoded_byte;// Temporary variable to hold one decoded byte
    unsigned char lsb[8];// Temporary variable to hold one decoded byte
    // Loop to decode 4 bytes (32 bits) representing the size
    for (int i = 0; i < 4; i++)
    {
        // Read 8 bytes from the stego image; if it fails, print error and return failure
        if (fread(lsb, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("error:To read encoded size from stego image\n");
            return e_failure;
        }
        // Decode a single byte from 8 LSBs; if it fails, return failure
        if (decode_byte_from_lsb((char *)lsb, &decoded_byte) != e_success)
        {

            return e_failure;
        }

        size = (size << 8) | (unsigned char)decoded_byte;//perform the shifting of 8bits data OR with decoded_byte
    }
    *decoded_size = size;// Store the final decoded size in the output variable
     return e_success;// Return success after decoding all 4 bytes
}
//decode file data
Status decode_file_data(DecodeInfo *decInfo, char *file_data)
{
    char data_block[8];  // buffer to read 8 bytes from stego image
    char decoded_byte;// Temporary variable to hold one decoded byte
    for (int i = 0; i < decInfo->size_secret_file; i++)
    {
        // Read 8 bytes from the stego image,if not means print error msg and return failure
        if (fread(data_block, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("error: not read encoded secret data from stego image\n");
            return e_failure;
        }
        // Decode 1 byte from the 8 bytes (LSB),if not means print error msg and return failure
        if (decode_byte_from_lsb(data_block, &decoded_byte) != e_success)
        {
            printf("error: decode_byte_from_lsb is failed\n");
            return e_failure;
        }
        file_data[i] = decoded_byte;// Store the final decoded byte in the fille  data
        file_data[i+1] = '\0';//null terminated
    }

    return e_success;

}
Status do_decoding(DecodeInfo *decInfo)
{
    //
    if (open_file(decInfo) != e_success)//open the stego_imag file
    {
        printf("Open_files_function_failed\n");//not opened meansprint error msg and it will be existed
        return e_failure;
    }
    printf("Open_files_function_is successfully\n");
    if (skip_bmp_header(decInfo->fptr_stego_image) != e_success)//verify the header is skipped or not
    {
        printf("skip the bmp header is failed\n");
        return e_failure;
    }
    printf("skipping the header is successfully\n");

    if (decode_magic_string(decInfo->fptr_stego_image) != e_success)//verify the magic string is decoded or not
    {
        printf("decode the magic string is failed\n");
        return e_failure;
    }
    printf("decode the magic string is successfully\n");

    unsigned int extn_size;
    if (decode_file_extn_size(decInfo, &extn_size) != e_success)//verify the file extension size is decoded or not
    {
        printf("decode the file extension size is failed\n");
        return e_failure;
    }
    printf("decode the file extension size is successfully\n");

    if (decode_file_extn(decInfo, extn_size) != e_success)//verify the file extension is decode or not
    {
        printf("decode the file extension is failed\n");
        return e_failure;
    }
    printf("decode the file extension is successfully\n");
    if (open_decode_files(decInfo) != e_success)//open the output file(default)
    {
        printf("open the decode files are failed\n");
        return e_failure;
    }
    printf("open the decode files are successfully\n");

    unsigned int secret_file_size;
    if (decode_file_size(decInfo, &secret_file_size) != e_success)//verify the file size is decoded or not
    {
        printf("decode the file_size  failed\n");
        return e_failure;
    }
    printf("decode the file_size  successfully\n");
    decInfo -> size_secret_file = secret_file_size;
    char secret_file_data[secret_file_size];    
    if (decode_file_data(decInfo,secret_file_data) != e_success)//verify the file data is decode or not
    {
        printf("decode the file_data is failed\n");
        return e_failure;
    }
    printf("decode the file_data successfully\n");
    if (fwrite(secret_file_data, 1, secret_file_size, decInfo->fptr_output_image) != secret_file_size)// Write the decoded data to output file
    {
        printf("error: failed to write decoded data to output file\n");
        return e_failure;
    }
    printf("decoded data written to output file successfully\n");

    return e_success;
}
