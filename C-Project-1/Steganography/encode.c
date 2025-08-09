//--------------------------------------------------------------------------------------//
//*DESCRIPTION:
//NAME:  Chintala Lavanya
//DATE:  23/05/2025
//PROJECT TITLE: LSB Image Steganography
//----------------------------------------------------------------------------------------//
#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"

//check if operation type is encode or not
OperationType check_operation_type(char *argv[])
{
    //check the encode argument
    if(strcmp(argv[1],"-e")==0)
    {
        return e_encode;
    }
    //check the decode argument
    else if(strcmp(argv[1],"-d")==0)
    {
        return e_decode;
    }
    else
    {
        return e_unsupported;
    }
}
//read and validate the encode args from argv
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Check if source image file has .bmp extension
    if (strstr(argv[2], ".bmp") != NULL)
    {
        encInfo->src_image_fname = argv[2]; // Assign source image filename
    }
    else
    {
        printf("ERROR: Source image must be a .bmp file\n"); // Invalid source file type
        return e_failure;
    }

    // Check if secret file has .txt extension
    if (strstr(argv[3], ".txt") != NULL)
    {
        encInfo->secret_fname = argv[3]; // Assign secret file name
        strcpy(encInfo->extn_secret_file, ".txt"); // Manually set file extension as .txt
    }
    else
    {
        printf("ERROR: Secret file must be a .txt file\n"); // Invalid secret file type
        return e_failure;
    }

    // Check if output image name is provided
    if (argv[4] != NULL)
    {
        // Check if output image has .bmp extension
        if (strstr(argv[4], ".bmp") != NULL)
        {
            encInfo->stego_image_fname = argv[4]; // Assign stego image filename
        }
        else
        {
            printf("error: Output image file is not .bmp\n"); // Not bmp, use default name
            encInfo->stego_image_fname = "stego_img.bmp";
        }
    }
    else
    {
        encInfo->stego_image_fname = "stego_img.bmp"; // Use default name
    }

    return e_success; // All arguments validated successfully
}
/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
//get the file size
uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END); //Moves the file pointer to the end of the file.
    uint size=ftell(fptr);//  get the current position of the file i.e size of file in bytes
    fseek(fptr, 0, SEEK_SET);//moves the file pointer to the end of the file
    return size; 
}
/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }
    // No failure return e_success
    return e_success;
}
//Copy bmp image header 
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    fseek(fptr_src_image,0,SEEK_SET);//moves back from begining of the file
    char header[54];//a buffer to store 54bytes of header
    fread(header,54,1,fptr_src_image);//read the header(54bytes) of source image file
    fwrite(header,54,1,fptr_dest_image);//write 54 bytes header into the dest image file
    return e_success;
}
//encode 1 byte to 8 bytes
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    for (int i = 0; i < 8; i++)
    {
        // Get the (7 - i)th bit of the data(MSB to LSB)
        char bit = (data >> (7 - i)) & 1;
        //Clear the LSB of the image byte (set it to 0)
        char cleared_byte = image_buffer[i] & 0xFE;
        //Set the LSB to the extracted bit
        image_buffer[i] = cleared_byte | bit;
    }
    return e_success;
}
//encode data to image
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char image_block[8]; //A buffer to store 8 bytes(8x8=64bits)of image data.
    for(int i = 0;i < size; i++)
    {
        fread(image_block,8,1,fptr_src_image);// Reads 8 bytes from the source image.
        encode_byte_to_lsb(data[i],image_block);//Encode each bit into the LSB of the image block bytes
        fwrite(image_block,8,1,fptr_stego_image);//writes the modified image_block to the stego_image
    }
    return e_success;
}
//encode  Magic String 
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    int length = strlen(magic_string); //calculate the length of magic string
    return encode_data_to_image((char*)magic_string,length,encInfo->fptr_src_image,encInfo->fptr_stego_image);//encodes the magic string into the source image to stego image
    
}
//encode secret file extension size
Status encode_secret_file_extn_size(int extn_size, EncodeInfo *encInfo)
{
    unsigned char lsb_block[8];//a buffer to store the 8bytes of size
    for(int i=0;i<4;i++)
    {
        unsigned char size_byte=(extn_size >> (24-i*8)) & 0xFF;// Extract the i-th byte from extn_size, starting from the MSB
        fread(lsb_block,8,1,encInfo->fptr_src_image);// Read 8 bytes from the source image into lsb_block
        encode_byte_to_lsb(size_byte,lsb_block);// hide the bits of size_byte into the LSB of lsb_block
        fwrite(lsb_block,8,1,encInfo->fptr_stego_image);// Write the modified 8-byte lsb_block into the stego image
    }
    return e_success;
}
// Encode secret file extenstion 
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    int extn_length=strlen(file_extn); //Calculates the length of the file extension
    return  encode_data_to_image((char*)file_extn,extn_length,encInfo->fptr_src_image,encInfo->fptr_stego_image);// encode the file extension into the source image to stego image
}
//Encode secret file size 
Status encode_secret_file_size(int file_size, EncodeInfo *encInfo)
{
    unsigned char lsb_block[8];
    for(int i=0;i<4;i++)
    {
        unsigned char size_byte=(file_size >> (24-i*8)) & 0xFF;// Extract the i-th byte from file_size, starting from the MSB
        fread(lsb_block,8,1,encInfo->fptr_src_image);// Read 8 bytes from the source image into lsb_block
        encode_byte_to_lsb(size_byte,lsb_block);// hide the bits of size_byte into the LSB of lsb_block
        fwrite(lsb_block,8,1,encInfo->fptr_stego_image);// Write the modified 8-byte lsb_block into the stego image
    }
    return e_success;
}
//Encode secret file data
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char ch;//ch variable to store a single byte of secret file data.
    char data_block[8];//to store the 8bytes image data
    for(long i=0;i<encInfo->size_secret_file;i++)//loop run upto the size of secret file
    {
        fread(&ch,1,1,encInfo->fptr_secret);//Reads a single byte from the secret file.
        fread(data_block,8,1,encInfo->fptr_src_image);//Reads 8 bytes from the source image
        encode_byte_to_lsb(ch,data_block);  //encode the ch into lsb of data_block
        fwrite(data_block,8,1,encInfo->fptr_stego_image);//writes the modified block into the stego_image
    }
    return e_success;
}
 // Copy remaining bytes
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    while (fread(&ch, 1, 1,fptr_src) > 0) //Reads one byte at a time from the source file (fptr_src)
    {
        fwrite(&ch, 1, 1, fptr_dest);//Writes that byte to the destination file (fptr_dest)
    }
    return e_success;
}

// Check if image has enough capacity
Status check_capacity(EncodeInfo *encInfo)
{
    long capacity_size = (strlen(MAGIC_STRING) + 4 + strlen(encInfo->extn_secret_file) + 4 + encInfo->size_secret_file) * 8;
    //check the image capacity is greater than or equal to capacity_size
    if (encInfo->image_capacity >= capacity_size)
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

// Perform encoding
Status do_encoding(EncodeInfo *encInfo)
{
    if (open_files(encInfo) != e_success)//open all files i.e.source image, secret file, stego image
    {
        return e_failure;//not opened means it will be existed
    }
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);//get the space in image for encoding and stores in encInfo->image_capacity
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);//gets the size of the secret file in bytes and stores it in encInfo->size_secret_file.

    if (check_capacity(encInfo) != e_success)//verify the image can hold the secret data
    {
        fprintf(stderr, "ERROR:There is no capacity to store the secret file data.\n");
        return e_failure;
    }

    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)//verify the header is copied or not
    {
        printf("failed to copy the header\n");
        return e_failure;
    }
    printf("copy bmp header successfully\n");

    if(encode_magic_string(MAGIC_STRING, encInfo) != e_success)//verify the magic string is encoded or not
    {
        printf("failed to encode the magic string\n");
        return e_failure;
    }
    printf("encoded magic string successfully\n");

    if(encode_secret_file_extn_size(strlen(encInfo -> extn_secret_file), encInfo) !=e_success)//verify the secret file extension is encoded or not
    {
       printf("failed to encode secret file extension size\n");
       return e_failure;
    }
    printf("encoded secret file extension size successfully\n");
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)//verift the secret file extension encode or not
    {
        printf("failed to encode secret file extension\n");
        return e_failure;
    }
     printf("encoded secret file extension successfully\n");
    if(encode_secret_file_size(encInfo->size_secret_file, encInfo) != e_success)//verify the secret file size is encoded or not
    {
        printf("failed to encode secret file size\n");
        return e_failure;
    }
    printf("encode secret file size successfully\n");
    if(encode_secret_file_data(encInfo) != e_success)//verify the secret file data encode or not
    {
       printf("failed to encode secret file data\n");
       return e_failure;
    }
    printf("encode secret file data successfully\n");
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)//verify the remaining data copied or not
    {
       printf("failed to copy the remaining data\n");
       return e_failure;
    }
    printf("copy the remaining data successfully\n");
    return e_success;
}






