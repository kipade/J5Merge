/*===========================================================================*\
 * FILE: checksum.c
 *===========================================================================
 * Copyright 2014 Delphi Technologies, Inc., All Rights Reserved.
 * Delphi Confidential
 *
 * DESCRIPTION:
 *
 * ABBREVIATIONS:
 *   TODO: List of abbreviations used, or reference(s) to external document(s)
 *
 * TRACEABILITY INFO:
 *   Design Document(s):
 *     TODO: Update list of design document(s)
 *
 *   Requirements Document(s):
 *     TODO: Update list of requirements document(s)
 *
 *   Applicable Standards (in order of precedence: highest first):
 *     SW REF 264.15D "Delphi C Coding Standards" [23-Dec-2001]
 *     TODO: Update list of other applicable standards
 *
 * DEVIATIONS FROM STANDARDS:
 *   TODO: List of deviations from standards in this file, or
 *   None.
 *
\*===========================================================================*/

/*===========================================================================*\
 * Standard Header Files
\*===========================================================================*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>
#include <ctype.h>
#include "common.h"
/*===========================================================================*\
 * Other Header Files
\*===========================================================================*/



/*===========================================================================*\
 * Local Preprocessor #define Constants
\*===========================================================================*/
/* store real length and crc value */
#define USED_BYTE_NUM_FROM_END 16
#define CEP 0x55AA55AA
//#define OUTPUT_FILE_SIZE 64

/*===========================================================================*\
 * Local Preprocessor #define MACROS
\*===========================================================================*/

/*===========================================================================*\
 * Local Type Declarations
\*===========================================================================*/

/*===========================================================================*\
 * External Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/
//uint32_t sum_all_4bytes_aligned(uint8_t * start_address, uint32_t length);
/*===========================================================================*\
 * Local Inline Function Definitions and Function-Like Macros
\*===========================================================================*/

/*===========================================================================*\
 * internal Function Definitions
\*===========================================================================*/
/*
uint32_t sum_all_4bytes_aligned(uint8_t * start_address, uint32_t length)
{
    uint32_t sum = 0;
    uint8_t* pointer = start_address;

    if(start_address == NULL || length == 0)
    {
        printf("Invalid parameter to calcalute checksum: start_address[%p], length[%u]\n",
                start_address, length);
        return EINVAL;
    }

    while(length >0)
    {
        sum += *pointer++;
        length -= 4;
    }

    return sum;
}
*/
static uint32_t sum_all_4bytes_aligned(uint8_t * start_address, uint32_t length)
{
   uint32_t sum = 0;
   uint32_t *pointer = (uint32_t*)start_address;
   if((start_address == 0) || (((ptr_int)start_address)%4 != 0) || (length%4 != 0))
   {
      return 0;
   } //if. no else

   while(length >0)
   {
      sum += *pointer++;
      length -= 4;
   } //while

   return sum;
}

void print_usage()
{
    printf("Please enter input filename, output filename, output filesize(Hex)and version!\n");
    printf("Example:checksum.exe uImage outImage 0x200000 v3.2.4\n");
}

int convert_version(const char* str, uint32_t* version_result)
{
    uint32_t version = 0;
    uint32_t input_len = strlen(str);
    uint32_t version_sep_count = 0;
    uint32_t pos = 0;

    if(str == NULL)
    {
        return 0;
    }

    while(pos < input_len && version_sep_count < 4)
    {
        int sub = 0;
        uint8_t temp = 0;
        if(str[pos] == '.')
        {
            version_sep_count ++;
            pos ++;
            continue;
        }
        while(sub < 2 && pos < input_len)
        {
            if(sub > 0 && str[pos] == '.')
            {
                ++version_sep_count;
                ++pos;
                break;
            }
            if(!isxdigit(str[pos]))
            {//不是一个合法的16进制字符
                printf("Input version string [%s] is invalid.\n", str);
                return -1;
            }
            if(isdigit(str[pos]))
            {
                temp = (temp << 4) + (uint8_t)(str[pos] - '0');
            }
            else if(islower(str[pos]))
            {
                temp = (temp << 4) + (uint8_t)(str[pos] - 'a' + 0x0A);
            }
            else
            {
                temp = (temp << 4) + (uint8_t)(str[pos] - 'A' + 0x0A);
            }

            sub++;
            pos++;
        }
        version = (version << 8) + temp;
    }//end while
    if(version_sep_count < 3)
    {
        version |= 0xFF000000;
    }

    if(version_result != NULL)
    {
        *version_result = version;
    }
    return 0;
}


int do_gen_checked_file(int argc, char *argv[])
{
    FILE* inputFile = NULL;
    FILE* outputFile = NULL;
    uint32_t nStart = 0;
    uint32_t nEnd = 0;
    uint32_t nLen = 0;
    uint32_t nCount = 0;
    uint32_t nOutputFileSize = 0;
    uint8_t * pInputBuff =NULL;
    uint32_t nChecksum = 0;
    uint32_t nVersion = 0;
    uint32_t nCodeExistPattern = CEP;
    int i = 0;
    int err = 0;
    int tail_size = USED_BYTE_NUM_FROM_END;

    if((argc < 5) || argv[1] == '\0' || argv[2] == '\0' || argv[3] == '\0' || argv[4] == '\0')
    {
        print_usage();
        return EINVAL;
    }
    inputFile = fopen(argv[1],"rb");
    if(NULL == inputFile)
    {
        err = errno;
        printf("can't open input file for read[%u]!\n", err);
        return err;
    }

    sscanf(argv[3],"%x",&nOutputFileSize);
    if(0 == nOutputFileSize)
    {
        printf("Read output file size error!\n");
        goto EXIT;
    }

    //calculate input file length
    nStart = ftell(inputFile);
    fseek(inputFile,0,SEEK_END);
    nEnd = ftell(inputFile);
    rewind(inputFile);
    nLen = nEnd - nStart;

    //make sure output file size if enough to store all the data
    if(argc > 4)
    {
        //the last parameter is sha
    }
    if((nLen + tail_size) > nOutputFileSize)
    {
        printf("The input file size[%d] is too larget to dump into output file size[%d]\n",
                nLen, nOutputFileSize);
        err = EINVAL;
        goto EXIT;
    }

    //malloc buffer
    pInputBuff = (uint8_t*) malloc(nLen);
    if(NULL == pInputBuff)
    {
        err = errno;
        printf("Malloc buffer failed[%u]!\n", err);
        goto EXIT;
    }

    //read whole file to buffer
    nCount = fread(pInputBuff,1,nLen,inputFile);
    if(nCount != nLen)
    {
        err = errno;
        printf("Read input file error[%u]!\n", err);
        goto EXIT;
    }

    //change version string to Hex format
    if(convert_version(argv[4]+1, &nVersion) != 0)
    {
        printf("Get Version info failed\n");
        err = EINVAL;
        goto EXIT;
    }

    //calculate Checksum
    /*
    nChecksum = nCodeExistPattern + nLen + nVersion + sum_all_4bytes_aligned(pInputBuff,nLen);
    nChecksum = ((~nChecksum) + 1);
    */
    nChecksum = sum_all_4bytes_aligned(pInputBuff,nLen);
    nChecksum += nCodeExistPattern + nLen + nVersion;
    nChecksum = -nChecksum;

    //open output file
    outputFile = fopen(argv[2],"wb");
    if(NULL == outputFile)
    {
        err = errno;
        printf("can't open output file for write[%u]!\n", err);
        goto EXIT;
    }

    //write file
    nCount = fwrite(pInputBuff,1,nLen,outputFile);
    if(nCount != nLen)
    {
        err = errno;
        printf("Writefile error[%u]!\n", err);
        goto EXIT;
    }

    //write pad 0xFF
    for(i = nLen; i<(int)nOutputFileSize - tail_size;i++)
    {
        fputc(0xFF,outputFile);
    }

    //write CEP, Len, Version, Checksum
    fwrite(&nCodeExistPattern,4,1,outputFile);
    fwrite(&nLen,4,1,outputFile);
    fwrite(&nVersion,4,1,outputFile);
    fwrite(&nChecksum,4,1,outputFile);
    printf("SectionEnd written: CEP: %08X, length: %08X, version: %08X, CRC: %08X\n", nCodeExistPattern, nLen, nVersion, nChecksum);

EXIT:
    if(NULL != inputFile)
    {
        fclose(inputFile);
    }

    if(NULL != outputFile)
    {
        fclose(outputFile);
    }

    if(NULL != pInputBuff)
    {
        free(pInputBuff);
        pInputBuff = NULL;
    }
    return err;
}

#if 0
#include <unistd.h>
int main(int argc, char* argv[])
{
#if 0
    char* cmds[] = {
        "checksum",
        "SMC.bin",
        "SMC",
        "0x100000",
        "v51.0.2.3"
    };
    const char* working_dir = "/home/kezhh/Work/Merged_Image";

	if(chdir(working_dir))
	{
        printf("enter %s failed[%u].\n", working_dir, errno);
	}
	do_gen_checked_file(sizeof(cmds)/sizeof(const char*), cmds);
#else
    int err = do_gen_checked_file(argc, argv);
#endif
	return err;
}

#endif
