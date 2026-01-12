#include <stdio.h>
#include<string.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include "show.h"

int main(int argc ,char*argv[])
{
    /* Check if user selected encoding */
    if(check_operation_type(argv) == e_encode)
    {
        EncodeInfo encInfo;                 // Structure for all encode information

        update_main_progress("ENCODING MODE SELECTED :", 0);

         /* Step 1 – Validate input arguments */
        if(read_and_validate_encode_args(argv,&encInfo) == e_success)
        {
            update_main_progress("ENCODING MODE SELECTED :", 5);
            show("INPUT ARGUMENTS VALIDATED SUCCESSFULLY...");

            /* Step 2 – Start encoding */
            if(do_encoding(&encInfo)==e_success)
            {
                update_main_progress("ENCODING MODE SELECTED :", 100);
                printf("ENCODING COMPLETED SUCCESSFULLY.\n");
            }
            else{
                printf("ENCODING FAILED.");
            }
        }
        else{
            printf("INPUT VALIDATION FAILED.\n");
        }
    }

    /* Check if user selected decoding */
    else if(check_operation_type(argv) == e_decode)
    {
        DecodeInfo decInfo;                  // Structure to hold all decode-related information

        update_main_progress("DECODING MODE SELECTED: ",0);

        /* Step 1 – Validate input arguments */
        if(read_and_validate_decode_args(argv,&decInfo) == e_success)
        {
            update_main_progress("DECODING MODE SELECTED: ",5);
            show("INPUT ARGUMENTS VALIDATED SUCCESSFULLY...");

            /* Step 2 – Perform decoding */
            if(do_decoding(&decInfo)==e_success)
            {
                update_main_progress("DECODING MODE SELECTED: ",100);
                printf("DECODING COMPLETED SUCCESSFULLY.\n");
            }
            else{
            printf("DECODING FAILED.\n");
            }
        }
        else{
            printf("READ AND VALIDATION FAILED.\n");
        }
        
    }
    else{
        printf("INVALID OPTION\n");
        printf("******************** USAGE ********************\n");
        printf("Encoding: ./a.out -e beatiful.bmp secret.txt stegno.bmp\n");
        printf("Decoding: ./a.out -d stegno.bmp \n");
    }

    return 0;
}


/*
 * Determines whether the user selected encoding or decoding mode.
 * Returns:
 *   e_encode       → if "-e" is passed
 *   e_decode       → if "-d" is passed
 *   e_unsupported  → for invalid or missing options
 */
OperationType check_operation_type(char *argv[]){
    if(strcmp(argv[1],"-e")==0) return e_encode;
    else if(strcmp(argv[1],"-d")==0) return e_decode;
    else  return e_unsupported;   
}


