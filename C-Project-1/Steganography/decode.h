#ifndef DECODE_H
#define DECODE_H

#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    uint image_capacity;
    uint bits_per_pixel;
    char image_data[MAX_IMAGE_BUF_SIZE];

    /* Secret File Info */
    char *secret_fname;
    FILE *fptr_secret;
    char extn_secret_file[MAX_FILE_SUFFIX];
    char secret_data[MAX_SECRET_BUF_SIZE];
    long size_secret_file;

    /* Stego Image Info */
    char *stego_image_fname;
    FILE *fptr_stego_image;

    //*duplicate image */
    FILE *fptr_output_image;
    char output_image_fname[20];
    char file_extn[MAX_SECRET_BUF_SIZE];

}DecodeInfo ;


/* Decoding function prototype */

//read and validate the decode args from argv
Status read_and_validate_decode_args(char *argv[],DecodeInfo *decInfo);

//open the files
Status open_file(DecodeInfo *encInfo);

//skip bmp image header 
Status skip_bmp_header(FILE *fptr_stego_image);

//decode byte from lsb
Status decode_byte_from_lsb(char *image_buffer, char *data);

//decode data from image
Status decode_data_from_image(FILE *fptr_stego_image, char *data, int size);

//decode  Magic String 
Status decode_magic_string(FILE *fptr_stego_image);

//decode file extension size
Status decode_file_extn_size(DecodeInfo *decInfo, unsigned int *extn_size);

//decode file extension
Status decode_file_extn(DecodeInfo *decInfo, unsigned int extn_size);

//open the decode files
Status open_decode_files(DecodeInfo *decInfo);

//decode file size
Status decode_file_size(DecodeInfo *decInfo, unsigned int *decoded_size);

//decode file data
Status decode_file_data(DecodeInfo *decInfo, char *file_data);

//do the decoding
Status do_decoding(DecodeInfo *decInfo);




#endif
