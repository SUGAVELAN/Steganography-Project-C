#include<stdio.h>
#include"types.h"
#include"decode.h"
#include<string.h>
#include<stdlib.h>
#include "common.h"
#include "show.h"



 // -------- Argument Validation & File Opening --------
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo){
    /* 
     * Argument Format:
     * ./a.out -d <stego.bmp>
     * argv[2] → stego image filename
     */

    // Check if argv[2] exists AND contains ".bmp"
    if (argv[2]!=NULL && strstr(argv[2],".bmp")!=NULL)
    {
        decInfo->decode_stego_fname = argv[2];
            return e_success; 
    } 
    
    else return e_failure;
}

Status open_stego_file(DecodeInfo* decInfo)
{
      /*
     * Step 1: Open the stego image file in read mode.
     * Step 2: Validate if the file opened successfully.
     */
    decInfo-> fptr_decode_stego= fopen(decInfo->decode_stego_fname, "r");
    
    if (decInfo->fptr_decode_stego == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n",decInfo->decode_stego_fname );

    	return e_failure;
    }
return e_success;
}
   
Status open_data_file(DecodeInfo* decInfo)
{    
    /*
     * Step 1: Open the output data file (extracted secret file).
     * Step 2: Validate if the file opened successfully.
     */

    decInfo->fptr_decode_scr = fopen( decInfo->decode_scr_fname, "w");
   
    if ( decInfo->fptr_decode_scr == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n",  decInfo->decode_scr_fname);

    	return e_failure;
    }


    return e_success;
}

//---------------------------------------------------------------------------------------------------------------------------------------------//

// -------- Core Decoding Helpers --------
unsigned char decode_char(FILE*fptr_decode_stego){
    unsigned char result = 0;
    unsigned char ch;

    /* Extract 8 LSB bits from 8 image bytes to form one character */
    for(int i = 0; i < 8; i++)
    {
        fread(&ch,1,1,fptr_decode_stego);
        result = result << 1;                  // Make space for next bit
        result = result | (ch&1);              // Extract LSB and store it
    }
    return result;
}

uint decode_int(FILE*fptr_decode_stego){
    uint value = 0;
    unsigned char ch;

    /* Extract 32 bits (LSB of 32 bytes) to reconstruct an integer */
    for(int i=0;i<32;i++){
        fread(&ch,1,1,fptr_decode_stego);
        value = value << 1;            // Shift to make space for next bit
        value = value | (ch&1);         // Extract LSB and store it
    }
    return value;
}
//------------------------------------------------------------------------------------------------------------------------------------------------//

// -------- Step-by-Step Decoding Operations --------
Status decode_magic_string(DecodeInfo *decInfo)
{
    
    /* Decode 2 characters (each extracted using decode_char) */
    for(int i=0;i<2;i++)
    {
        decInfo->decode_magic_str[i] = decode_char(decInfo->fptr_decode_stego);
    }
   decInfo->decode_magic_str[2]='\0';      // Null-terminate the string
   return e_success;
}

/* Compare extracted magic string with the expected MAGIC_STRING */
Status compare_magic_string(DecodeInfo *decInfo)
{
   if(strcmp(decInfo->decode_magic_str,MAGIC_STRING)==0)
   {
    return e_success;
   }
   else{
    return e_failure;
   }

}


Status decode_extn_size(DecodeInfo * decInfo)
{
    /* Decode 32-bit extension size from the stego image */
   decInfo->scr_extn_size = decode_int(decInfo->fptr_decode_stego);
   return e_success;
}

Status decode_extn(DecodeInfo * decInfo)
{
    /* Allocate memory for extension string (+1 for null terminator) */
    decInfo->decode_scr_extn_name =  malloc(decInfo->scr_extn_size + 1);
     if (decInfo->decode_scr_extn_name == NULL)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for extension name\n");
        return e_failure;
    }

    uint i;
    /* Decode each character of the extension */
    for(i=0;i<decInfo->scr_extn_size;i++)
    {
     decInfo->decode_scr_extn_name[i] =decode_char(decInfo->fptr_decode_stego);
    }

     /* Null-terminate the decoded extension string */
     decInfo->decode_scr_extn_name[decInfo->scr_extn_size]='\0';

   // printf("EXTENSION TYPE : %s\n",decInfo->decode_scr_extn_name);
    return e_success;
}


Status create_data_file(DecodeInfo * decInfo)
{
    /* Length of "DATA" + extension + null terminator */
    int len = 4 + decInfo->scr_extn_size + 1;

    /* Allocate memory for output filename */
    decInfo->decode_scr_fname = malloc(len);
    if (decInfo->decode_scr_fname == NULL)
    {
        fprintf(stderr, "ERROR: Memory allocation failed for output filename\n");
        return e_failure;
    }

     /* Build the output file name */
    strcpy(decInfo->decode_scr_fname,"DATA");
    strcat(decInfo->decode_scr_fname,decInfo->decode_scr_extn_name);


    //printf("OUTPUT FILE NAME: %s\n", decInfo->decode_scr_fname);
    /* Create (open) the output file */
    if(open_data_file(decInfo)==e_success)
    {
        return e_success;
    }
    return e_failure;
}


Status decode_data_file_size(DecodeInfo * decInfo)
{
    /* Read the next 32 bits as the secret data size */
   decInfo->stego_data_size = decode_int(decInfo->fptr_decode_stego);

    /* Validation: size should not be zero or suspiciously large */
    if (decInfo->stego_data_size == 0)
    {
        fprintf(stderr, "ERROR: Decoded data size is zero\n");
        return e_failure;
    }
   //printf("DATA SIZE : %u bytes\n",decInfo->stego_data_size);
   return e_success;
}


Status decode_data(DecodeInfo * decInfo)
{
/* Allocate buffer for decoded secret data */
char *DATA =malloc(decInfo->stego_data_size+1);

if (DATA == NULL)
    {
        printf("ERROR: Memory allocation failed for secret data\n");
        return e_failure;
    }

/* Decode secret bytes from stego image */
for(uint i=0;i<decInfo->stego_data_size;i++){
    DATA[i]=decode_char(decInfo->fptr_decode_stego);
}

/* Write decoded data into output file */
fwrite(DATA,1,decInfo->stego_data_size,decInfo->fptr_decode_scr);

/* Free allocated memory */
free(DATA);
return e_success;
}
//------------------------------------------------------------------------------------------------------------------------------------------------//

// -------- Main Decoding Controller --------
Status do_decoding(DecodeInfo *decInfo)
{
    if(open_stego_file(decInfo)==e_success)
    {
        update_main_progress("DECODING MODE SELECTED: ",10);
        show("STEGO FILE OPENED SUCCESSFULLY...");

        fseek(decInfo->fptr_decode_stego,54,SEEK_SET);
        if(decode_magic_string(decInfo)==e_success)
        {
            update_main_progress("DECODING MODE SELECTED: ",20);
            show("MAGIC STRING DECODED SUCCESSFULLY...");

            if(compare_magic_string(decInfo)==e_success)
            {
                update_main_progress("DECODING MODE SELECTED: ",30);
                show("MAGIC STRING MATCHED SUCCESSFULLY...");

                if(decode_extn_size(decInfo)==e_success)
                {
                    update_main_progress("DECODING MODE SELECTED: ",40);
                    show("EXTENSION SIZE DECODED SUCCESSFULLY...");

                    if(decode_extn(decInfo)==e_success)
                    {
                        update_main_progress("DECODING MODE SELECTED: ",45);
                        show("FILE EXTENSION DECODED SUCCESSFULLY...");

                        if(create_data_file(decInfo)==e_success)
                        {
                            update_main_progress("DECODING MODE SELECTED: ",65);
                            show("DATA FILE CREATED SUCCESSFULLY...");

                            if(decode_data_file_size(decInfo)==e_success)
                            {
                                update_main_progress("DECODING MODE SELECTED: ",80);
                                show("DATA FILE SIZE DECODED SUCCESSFULLY...");

                                if(decode_data(decInfo)==e_success)
                                {
                                    update_main_progress("DECODING MODE SELECTED: ",95);
                                    show("DATA DECODED SUCCESSFULLY...");
                                }
                                else{
                                    printf("FAILED TO DECODE DATA\n");
                                    return e_failure;
                                }

                            }
                            else{
                                printf("FAILED TO DECODE DATA FILE SIZE\n");
                                return e_failure;
                            }

                        }
                        else{
                            printf("FAILED TO CREATE DATA FILE\n");
                            return e_failure;
                        }

                    }
                    else{
                        printf("FAILED TO DECODE FILE EXTENSION\n");
                        return e_failure;
                    }

                }
                else{
                    printf("FAILED TO DECODE EXTENSION SIZE\n");
                }
            }
            else{
                printf("MAGIC STRING NOT MATCHED\n");
                return e_failure;
            }
        }
        else{
            printf("FAILED TO DECODE MAGIC STRING\n");
            return e_failure;
        }
    }
    else{
        printf("FAILED TO  OPEN STEGO FILE\n");
        return e_failure;
    }
    free(decInfo->decode_scr_extn_name);
    free(decInfo->decode_scr_fname);
    fclose(decInfo->fptr_decode_stego);
    fclose(decInfo->fptr_decode_scr);
   return e_success;

}