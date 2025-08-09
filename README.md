## PROJECT TITLE:  **Image Steganography using LSB Encoding and Decoding**

## Description

     -This project implements Least Significant Bit (LSB) steganography for hiding and retrieving data inside a BMP image.  
     -It allows embedding any file into an image without visibly altering the image.
     
## Files in This Project

- **beautiful.bmp** – Original cover image used for encoding  
- **stego_img.bmp** – Image with hidden data after encoding  
- **secret.txt** – Example file to hide  
- **output.txt** – Extracted secret file after decoding  
- **encode.c / encode.h** – Encoding logic  
- **decode.c / decode.h** – Decoding logic  
- **common.h** – Common definitions and helpers  
- **types.h** – Type definitions and enums  
- **test_encode.c** – Test program for encoding

## Features

- Use the tool through command-line to hide or find messages.
- Checks if the image is big enough before hiding data.
- Keeps the image looking the same without visible changes.
- Lets you choose a name for the output file or uses a default name.
- Shows errors if files are missing or wrong.

**USAGE**

### Encoding
./a.out -e <SOURCE_IMAGE.bmp> <SECRET_FILE> [OUTPUT_STEGO_IMAGE.bmp]

- <SOURCE_IMAGE.bmp> Path to the 24‑bit BMP carrier image.
- <SECRET_FILE> File to embed (any type).
- [OUTPUT_STEGO_IMAGE.bmp] (optional) Name of the generated stego image. Defaults to stego.bmp.


Example

./a.out -e beautiful.bmp secret.txt stego_img.bmp

### Decoding

./a.out -d <STEGO_IMAGE.bmp> [OUTPUT_BASE_NAME]
- <STEGO_IMAGE.bmp> BMP containing hidden data.
- [OUTPUT_BASE_NAME] (optional) Base name for the recovered file; original extension is appended. Defaults to secret.

Example

./a.out -d stego_img.bmp output.txt

**How It Works**

1.Opened the carrier BMP image in binary read mode

   - Used fopen() to read raw binary pixel data from the 24-bit uncompressed BMP file.

2.Copied the 54-byte BMP header unchanged to the output stego image Preserved file structure so that the stego image remains a valid, viewable BMP.

3.Checked available storage capacity

   - Read width and height from BMP header.

   - Calculated total bits available:

    - pixels = width × height
    - total_bytes = pixels × 3
    - capacity_bits = total_bytes
    
  - Verified that metadata + payload size fits into the available bits.

4. Stored metadata before payload

   - Magic string (password) length + characters.

   - Original file extension length + characters.

   - File size as a 4-byte little-endian integer.

5. Encoded secret file byte-by-byte into image pixel bytes’ least significant bits

    - For each bit of the secret data:

    - Read 1 byte from the BMP pixel data.

    - Replaced its least significant bit with the secret bit.

    - Wrote the modified byte to the output stego image.

6. Copied remaining BMP data unchanged after the payload
    - Ensured that unused pixel data and padding remain intact.

7. Saved the modified BMP as the stego image
    - This image looks identical to the original but contains the hidden file.

8. Decoding process (reverse of encoding)

     - Opened stego BMP in binary read mode.

     - Skipped 54-byte header.

     - Read metadata (magic string, extension, size) from LSBs.

     - Verified magic string matches.

     - Read payload bits from LSBs and reconstructed the original file byte-by-byte.

    - Saved the recovered file with its original extension.

## Author

Lavanya Chintala
Email: chintalalavanya634@gmail.com



