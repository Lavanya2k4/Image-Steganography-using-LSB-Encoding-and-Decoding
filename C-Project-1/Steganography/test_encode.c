#include <stdio.h>
#include "encode.h"
#include "types.h"
#include "decode.h"

int main(int argc, char *argv[])
{
    EncodeInfo encInfo;
    DecodeInfo decInfo;
    // Check if at least one argument is passed
    if (argc < 2)
    {
        printf("./a.out -e <source bmp> <secret file> <output bmp>\n");
        printf("./a.out -d <source bmp> <output file>\n");
        return 1;
    }
    //check the operation type is equal to the e_encode or not
    if (check_operation_type(argv) == e_encode)
    {
        //check the aguments of encoding
        if (argc < 4)
        {
            printf("Use:./a.out -e <source bmp> <secret file> <output bmp>\n");
            return 1;
        }
        //read and validate the encode arguments
        if (read_and_validate_encode_args(argv, &encInfo) == e_success)
        {
            //check the do encoding 
            if (do_encoding(&encInfo) == e_success)
            {
                printf("Encoding completed successfully\n");
            }
            else
            {
                printf("Encoding failed\n");
            }
        }
        else
        {
            printf("Please enter valid encoding arguments\n");
        }
    }
    //check operation type is equal to e_decode or not
    else if (check_operation_type(argv) == e_decode)
    {
        //check the arguments
        if (argc < 3)
        {
            printf("Use: ./a.out -d <source bmp> <output file>\n");
            return 1;
        }
        //read and validate the decode arguments
        if (read_and_validate_decode_args(argv, &decInfo) == e_success)
        {
            //check the do_decoding
            if (do_decoding(&decInfo) == e_success)
            {
                printf("Decoding completed successfully\n");
            }
            else
            {
                printf("Decoding failed\n");
            }
        }
        else
        {
            printf("Please enter valid decoding arguments\n");
        }
    }
    else
    {
        printf("Invalid operation type. Use -e for encoding or -d for decoding.\n");
    }

    return 0;
}
