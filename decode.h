#ifndef DECODE_H
#define DECODE_H

#include "types.h"


/* -----------------------------------------------------------
 * Structure to hold all decode-related information
 * -----------------------------------------------------------*/

typedef struct _decodeInfo
{

/* ---- Stego Image Information ---- */
char *decode_stego_fname;        // Name of stego BMP file
FILE *fptr_decode_stego;        // File pointer to stego BMP
uint scr_extn_size;            // Extension length (decoded)
char *decode_scr_extn_name;   // Extracted extension string
char*stego_image_extn_name;    
uint stego_data_size;        // Size of secret data



char decode_magic_str[3];     // "#*" magic string

/* ---- Output Secret File Info ---- */
char *decode_scr_fname;        // Output file name (DATA.txt)
FILE *fptr_decode_scr;         // Pointer to output file

}DecodeInfo;

/* -----------------------------------------------------------
 * Function Declarations
 * -----------------------------------------------------------*/

 // -------- Argument Validation & File Opening --------
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status open_stego_file(DecodeInfo* decInfo);
Status open_data_file(DecodeInfo* decInfo);

// -------- Core Decoding Helpers --------
unsigned char decode_char(FILE*fptr_decode_stego);
uint decode_int(FILE*fptr_decode_stego);

// -------- Step-by-Step Decoding Operations --------
Status decode_magic_string(DecodeInfo *decInfo);
Status compare_magic_string(DecodeInfo *decInfo);

Status decode_extn_size(DecodeInfo * decInfo);
Status decode_extn(DecodeInfo * decInfo);

Status create_data_file(DecodeInfo * decInfo);

Status decode_data_file_size(DecodeInfo * decInfo);
Status decode_data(DecodeInfo * decInfo);

// -------- Main Decoding Controller --------
Status do_decoding(DecodeInfo *decInfo);

#endif