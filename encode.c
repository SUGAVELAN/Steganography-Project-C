#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "types.h"
#include "common.h"
#include "show.h"
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
   // printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    //printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
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

/*  
 * STEP 1:
 * Read command-line arguments and update the EncodeInfo structure.
 * Expected usage:
 *      ./a.out -e <src.bmp> <secret.txt> [stego.bmp]
 */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo){
    int flag=0;

    /* Validate source BMP file */
    if (argv[2]!=NULL && strstr(argv[2],".bmp")!=NULL){
        encInfo->src_image_fname = argv[2];
            flag=1;
    } 
    else return e_failure;

    /* Validate secret text file */
    if (argv[3]!=NULL && strstr(argv[3],".txt")!=NULL) {
        flag=1;
        encInfo->secret_fname = argv[3];
    }
    else return e_failure;
     
    /* Optional output stego filename */
    if(argv[4]!=NULL){
        encInfo->stego_image_fname=argv[4]; 
    }
    else{
        encInfo->stego_image_fname="stego.bmp";
    
    }
    if(flag == 1) return e_success; 
}


/*
 * Get the size of the secret file in bytes.
 * Uses fseek + ftell to move to the end and measure the size.
 */
uint get_file_size(FILE*fptr_secret)
{    

    /* Move file pointer to the end */                                                
    fseek(fptr_secret,0,SEEK_END);                       //without loop, moving the file pointer to last character 
    
    /* Get current position -> file size */
    uint size = ftell(fptr_secret);                       //ftell will return the current index.

    /* Reset file pointer back to start */
    fseek(fptr_secret,0,SEEK_SET);

     return size;                
}


/*
 * Check if the source image has enough capacity to hide:
 *  - 2 bytes  : Magic string ("#*")
 *  - 4 bytes  : Extension size
 *  - N bytes  : Extension name (".txt", ".png", etc.)
 *  - 4 bytes  : Secret file data size
 *  - X bytes  : Secret file data itself
 *
 * Each byte requires 8 image bytes (1 bit per byte stored using LSB).
 */

Status check_capacity(EncodeInfo *encInfo)
{
    /* Total encodable capacity of image (in bytes) */
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    /* Secret file size in bytes */
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

    uint required_bytes = 
            (2      /* magic string size */
            + 4     /* extension size (uint) */
            + 4     /* extension name (".txt" typically 4 bytes) */
            + 4     /* secret file size (uint) */
            + encInfo->size_secret_file);

    uint required_image_capacity = required_bytes * 8;

    /* Compare */
    if(encInfo->image_capacity > required_image_capacity)
    {         
        return e_success;
    }
    else return e_failure;
}


/*
 * Copies the first 54 bytes (BMP header) from the source BMP
 * to the stego BMP. These 54 bytes are fixed for all BMP images.
 */
Status copy_bmp_header(FILE*fptr_src_image,FILE*fptr_stego_image)
{
    char header[54];
    /* Move file pointer to beginning of BMP */
    fseek(fptr_src_image,0,SEEK_SET);

    /* Read the standard 54-byte BMP header */
    size_t bytes_read = fread(header,sizeof(char),54,fptr_src_image);

    /* Safety check */
    if (bytes_read != 54)
        return e_failure;

    /* Write the same header to the output (stego) image */
    fwrite(header,54,sizeof(char),fptr_stego_image);
    return e_success;
}


/* 
 * Encodes a string of bytes into image LSBs.
 * Each character requires 8 bytes from the image.
 */
Status encode_data_to_image(const char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image,EncodeInfo *encInfo)
{
      for(int i=0;i<size;i++)
      {
        /* Read 8 bytes (RGB values) from the source image[beatiful.bmp] */
        fread(encInfo->image_data,8,sizeof(char),encInfo->fptr_src_image);

        /* Encode 1 byte of secret data into these 8 bytes */
        encode_byte_to_lsb(data[i],encInfo->image_data);

        /* Write the modified 8 bytes to the output stego image */
        fwrite(encInfo->image_data,8,sizeof(char),encInfo->fptr_stego_image);
      }
    return e_success;
}


/*
 * Encodes 1 byte (8 bits) into 8 bytes of image data.
 * Each bit of 'data' goes into the LSB of one image byte.
 */

Status encode_byte_to_lsb(char data, char *image_buffer)
{
    unsigned char mask = 1 << 7;    // 10000000 — used to extract bits from MSB to LSB
    for(int i=0;i<8;i++)
    {
        /* Clear LSB & insert the current bit of 'data' */
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7-i));
        mask = mask >> 1;            // Move mask to next bit
    }
    return e_success;
}


/*
 * Encodes the magic string into the stego image.
 * This acts as an identifier so that the decoder can verify
 * whether the image contains hidden data or not.
 */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    /* Encode the MAGIC_STRING into the image using LSB substitution */
    encode_data_to_image(magic_string,strlen(MAGIC_STRING),encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo);
    return e_success;
}



/*
 * Reads 32 bytes from the source image, embeds the 32-bit SIZE into them
 * using encode_size_to_lsb(), and writes the modified bytes to the
 * stego image.
 */
Status encode_size(uint SIZE,FILE*fptr_src,FILE*fptr_stego)
{
    char buffer[32];

    // Read 32 bytes from source image
    fread(buffer,32,sizeof(char),fptr_src);

    // Encode the integer into those bytes
    encode_size_to_lsb(buffer,SIZE);

    // Write modified bytes into stego image
    fwrite(buffer,32,sizeof(char),fptr_stego);

    return e_success;
}


/*
 * Encodes a 32-bit integer (SIZE) into the LSBs of 32 bytes of image data.
 * Each byte carries 1 bit of the integer.
 */
Status encode_size_to_lsb(char*image_buffer,uint SIZE)
{
    uint  mask = 1 << 31;         // Start with MSB of 32-bit integer
    for(int i=0;i<32;i++)
    {

        image_buffer[i]= (image_buffer[i] & 0xFE) |      // Clear LSB
                         ((SIZE & mask) >> (31-i));      // Insert the bit
        mask = mask >> 1;                                // Move to next bit
    }
    return e_success;
}


/*
 * Encodes the secret file extension (e.g., ".txt") into the image.
 * This simply forwards the extension string to encode_data_to_image(),
 * which hides each character by modifying LSBs of 8 bytes per character.
 */
Status encode_secret_file_extn(const char*extn,EncodeInfo *encInfo)
{
    
    encode_data_to_image(extn,strlen(extn),encInfo->fptr_src_image,encInfo->fptr_stego_image,encInfo);
    return e_success;
}


/*
 * Encodes the size of the secret file (in bytes) into the stego image.
 * The size is encoded as a 32-bit integer, using 32 bytes of pixel data.
 */
Status encode_secret_file_size(long int SIZE,EncodeInfo *encInfo)
{
    encode_size(SIZE,encInfo->fptr_src_image,encInfo->fptr_stego_image);
    return e_success; 
}

/*
 * Encodes the actual secret file data into the stego image.
 * Each character from the secret file is embedded using 8 bytes
 * of image data (one bit per byte).
 */
Status encode_secret_file_data(EncodeInfo* encInfo)
{
    char ch;
    for(int i=0;i<encInfo->size_secret_file;i++)
    {
        // Read one character from the secret file
        fread(&ch,1,sizeof(char),encInfo->fptr_secret);

        // Read 8 bytes (1 pixel row) from the source image
        fread(encInfo->image_data,8,sizeof(char),encInfo->fptr_src_image);

        // Encode this character into the 8 bytes (LSB Steganography)
        encode_byte_to_lsb(ch,encInfo->image_data);

        // Write the modified bytes into the stego image
        fwrite(encInfo->image_data,8,sizeof(char),encInfo->fptr_stego_image);
         
    }
    return e_success;
    
}


/*
 * Copies all the remaining image data from the source BMP file
 * to the output stego BMP file.
 *
 * This is required because only part of the image is used for
 * embedding secret information. The rest of the BMP pixel data
 * must remain unchanged to preserve image integrity.
 */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest) 
{
    char ch;
    // Copy byte-by-byte until end of file
    while(fread(&ch,sizeof(char),1,fptr_src)>0)         //fread() returns the number of blocks read
    {                                                  //fread() returns 0 → EOF → loop stops
        fwrite(&ch,sizeof(char),1,fptr_dest);          //fread/write(pointer, size_of_each_block, number_of_blocks, file_ptr);
    }
    return e_success;
}


Status do_encoding(EncodeInfo *encInfo)
{
        if(open_files(encInfo)==e_success)
        {
            update_main_progress("ENCODING MODE SELECTED :", 10);
            show("FILE OPENED SUCCESSFULLY...");

            if (check_capacity(encInfo)==e_success)
            {
                update_main_progress("ENCODING MODE SELECTED :", 20);
                show("IMAGE HAS SUFFICIENT SPACE TO HIDE THE SECRET DATA...");

                if(copy_bmp_header(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
                {
                    update_main_progress("ENCODING MODE SELECTED :", 30);
                    show("HEADER COPIED SUCCESSFULLY...");

                    if(encode_magic_string(MAGIC_STRING, encInfo)==e_success)
                    {
                        update_main_progress("ENCODING MODE SELECTED :", 40);
                        show("MAGIC STRING ENCODED SUCCESSFULLY...");

                        char *ext = strchr(encInfo->secret_fname, '.');
                        if(encode_size(strlen(ext),encInfo->fptr_src_image,encInfo->fptr_stego_image) == e_success)
                        {
                            update_main_progress("ENCODING MODE SELECTED :", 45);
                            show("SECRET FILE EXTENSION SIZE ENCODED SUCCESSFULLY...");

                            if(encode_secret_file_extn(ext,encInfo)==e_success)
                            {
                                update_main_progress("ENCODING MODE SELECTED :", 50);
                                show("SECRET FILE EXTENSION ENCODED SUCCESSFULLY...");

                                if(encode_secret_file_size(encInfo->size_secret_file,encInfo)==e_success)
                                {
                                    update_main_progress("ENCODING MODE SELECTED :", 55);
                                    show("SECRET FILE SIZE ENCODED SUCCESSFULLY...");

                                    if(encode_secret_file_data(encInfo)==e_success)
                                    {
                                        update_main_progress("ENCODING MODE SELECTED :", 60);
                                        show("SECRET FILE DATA ENCODED SUCCESSFULLY...");

                                        if(copy_remaining_img_data(encInfo->fptr_src_image,encInfo->fptr_stego_image)==e_success)
                                        {
                                            update_main_progress("ENCODING MODE SELECTED :", 80);
                                            show("REMAINING PART COPIED SUCCESSFULLY...");
                                        }
                                        else{
                                        printf("FAILED TO COPY REMAINING PART\n");
                                        return e_failure;
                                        }
                                    }
                                    else{
                                        printf("FAILED TO ENCODE SECRET FILE DATA\n");
                                        return e_failure;
                                    }
                                }
                                else{
                                    printf("FAILED TO ENCODE SECRET FILE SIZE\n");
                                    return e_failure;
                                }
                            }
                            else{
                                printf("FAILED TO ENCODE SECRET FILE EXTENTION \n");
                                return e_failure;
                            }
                        }
                        else{
                            printf("FAILED TO ENCODE SECRET FILE EXTENSION SIZE\n");
                            return e_failure;
                        }
                    }
                    else{
                        printf("FAILED TO ENCODE MAGIC STRING\n");
                        return e_failure;
                    }
                }
                else {
                    printf("HEADER COPYED FAILED\n");
                    return e_failure;
                }
            }
            else{
                printf("IMAGE SPACE NOT ENOUGHF!!\n");
                return e_failure;
            }
        }
        else{
            printf("FAILD TO OPEN THE FILE\n");
            return e_failure;
        }



fclose(encInfo->fptr_src_image);
fclose(encInfo->fptr_secret);
fclose(encInfo->fptr_stego_image);

return e_success;
}