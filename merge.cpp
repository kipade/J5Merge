/*===========================================================================*\
 * FILE: Merge_image.c
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
#include "check_flash_sections.h"
#include "merge.h"
/*===========================================================================*\
 * Other Header Files
\*===========================================================================*/



/*===========================================================================*\
 * Local Preprocessor #define Constants
\*===========================================================================*/
#define FILE_NAME_SIZE 0x20
#define BUFFER_SIZE 0x440
#define READ_WRITE_SIZE 0x400 //1k
#define FLASH_SIZE 0x2000000  //32M
#define RESERVED_SIZE 0x400   //1k
#define FLASH_MEMORY_MAP_START_ADDRESS    0x1FFF800 //1k size
#define FLASH_MEMORY_MAP_END_ADDRESS    0x1FFFC00

#define ERROR -1

/* store real length and crc value */
#define USED_BYTE_NUM_FROM_END 16
#define CEP 0x55AA55AA
#define FLASH_SECTION_MAX_NUMBER 0x20
#define FLASH_STRUCTRUE_REVISION (unsigned int)0x00010000

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
unsigned int check_sum(unsigned char * start_address, unsigned int length, unsigned int init_checksum);
bool check_binary(const char* path);
/*
typedef struct Flash_Memory_Map_Tag
{
	unsigned int nSegmentId;
	unsigned int nSegmentStartAddress;
	unsigned int nSegmentEndAddress;
} Flash_Memofy_Map_T;
*/
/*===========================================================================*\
 * Local Inline Function Definitions and Function-Like Macros
\*===========================================================================*/

/*===========================================================================*\
 * internal Function Definitions
\*===========================================================================*/
unsigned int check_sum(unsigned char * start_address, unsigned int length, unsigned int init_checksum)
{
    unsigned int checksum = init_checksum;
    unsigned int *pointer = (unsigned int*)start_address;
    if((start_address == 0) || (((ptr_int)start_address)%4 != 0) || (length%4 != 0))
    {
        return 0;
    }

    while(length >0)
    {
        checksum += *pointer++;
        length -= 4;
    }

    return ((~checksum) + 1);
}

unsigned int currentAddress = 0;

int read_and_write_file(FILE* inputFile, FILE* outputFile,unsigned char * pInputBuff)
{
	unsigned int nCountRead = 0;
	unsigned int nCountWrite = 0;
	unsigned int nLen = 0;

	while(1)
	{
		nCountRead = fread(pInputBuff,1,READ_WRITE_SIZE ,inputFile);
		nCountWrite = fwrite(pInputBuff,1,nCountRead,outputFile);
		if(nCountWrite != nCountRead)
		{
			printf("\nWritefile error!\n");
			return ERROR;
		}
		else
		{
			currentAddress += nCountWrite;
			nLen += nCountWrite;
		}

		if(nCountWrite < READ_WRITE_SIZE)
		{
			break; //finished one file
		}
	}

	return 0;
}
#ifdef NOCONFIG_FILE_SUPPORT
int do_merge(std::vector<SectionItem>& sections, const char* output_file)
{
	FILE* inputFile = NULL;
	FILE* outputFile = NULL;
	unsigned char * pInputBuff =NULL;
	int ret =0 ;
	const char* fileName;
	unsigned int startAddress = 0,endAddress = 0;
	Flash_Memofy_Map_T flash_memory_map[FLASH_SECTION_MAX_NUMBER];
	unsigned int section_number = 0;
	unsigned int section_id = 0;

	unsigned int nVersion = FLASH_STRUCTRUE_REVISION;
	unsigned int nCodeExistPattern = CEP;
	unsigned int nLen = 0;
	const char* output_file_path = output_file;

	if(sections.size() == 0)
        return 0;

	if(output_file_path == NULL)
	{
        output_file_path = "bev_nor_flash.bin";
    }
	currentAddress = 0;

	printf("start!\n");

	/*const char* working_dir = "/home/kezhh/Work/Merged_Image";

	if(chdir(working_dir))
	{
        printf("enter %s failed[%u].\n", working_dir, errno);
	}*/

	pInputBuff = (unsigned char*) malloc(BUFFER_SIZE);
	if(NULL == pInputBuff)
	{
		printf("Malloc buffer failed!\n");
		ret = -EINVAL;
		goto EXIT;
	}

	outputFile = fopen(output_file_path,"wb");
	if(NULL == outputFile)
	{
		printf("can't open %s for write!\n", output_file_path);
		ret = -EINVAL;
		goto EXIT;
	}


	//while(1)
	for(int i = 0; i <= sections.size(); ++i)
	{
		if(i == sections.size())
		{
			unsigned int nCrc = 0;
			unsigned int nWrited = 0;
			//write flash memory map into the end, and reserve RESERVED_SIZE space
			//attach 0xFF as pad
			while(currentAddress < FLASH_MEMORY_MAP_START_ADDRESS)
			{
				fputc(0xFF,outputFile);
				currentAddress++;
			}

			//write flash memory map section number
			nWrited = fwrite(&section_number,1,sizeof(section_number),outputFile);
			if(nWrited != sizeof(section_number))
			{
				printf("\nWritefile error!\n");
				ret = -EINVAL;
				goto EXIT;
			}
			currentAddress += sizeof(section_number);

			//write flash memory map each sections
			nWrited = fwrite(flash_memory_map,1,sizeof(Flash_Memofy_Map_T) * section_number,outputFile);
			currentAddress += (sizeof(Flash_Memofy_Map_T) * section_number);
			if(nWrited != (sizeof(Flash_Memofy_Map_T) * section_number))
			{
				printf("\nWritefile error!\n");
				ret = -EINVAL;
				goto EXIT;
			}

			if(currentAddress > FLASH_MEMORY_MAP_END_ADDRESS)
			{
				printf("\nFlash Memory Map Write error!\n");
				ret = -EINVAL;
			}

			//attach 0xFF as pad
			while(currentAddress < FLASH_MEMORY_MAP_END_ADDRESS - USED_BYTE_NUM_FROM_END)
			{
				fputc(0xFF,outputFile);
				currentAddress++;
			}

			nLen = sizeof(section_number) + sizeof(Flash_Memofy_Map_T) * section_number;

			nCrc = nCodeExistPattern + nLen + nVersion + section_number;
			nCrc = check_sum((unsigned char *)flash_memory_map,sizeof(Flash_Memofy_Map_T) * section_number, nCrc);

			//write CEP, Len, Version, CRC
			fwrite(&nCodeExistPattern,4,1,outputFile);
			currentAddress += 4;
			fwrite(&nLen,4,1,outputFile);
			currentAddress += 4;
			fwrite(&nVersion,4,1,outputFile);
			currentAddress += 4;
			fwrite(&nCrc,4,1,outputFile);
			currentAddress += 4;

			//attach 0xFF as pad
			while(currentAddress < FLASH_SIZE)
			{
				fputc(0xFF,outputFile);
				currentAddress++;
			}

			printf("Finished!\n");
			break; //finished
		}
		fileName = sections[i].dst_file.c_str();
		section_id = sections[i].id;

        startAddress = sections[i].start_pos;
		printf("s:0x%-7x ",startAddress);

		endAddress = sections[i].end_pos;
		printf("e:0x%-7x\n",endAddress);

		flash_memory_map[section_number].nSegmentId = section_id;
		flash_memory_map[section_number].nSegmentStartAddress = startAddress;
		flash_memory_map[section_number].nSegmentEndAddress = endAddress;
		section_number ++;

		if((startAddress != currentAddress) || (endAddress > FLASH_SIZE))
		{
			printf("\naddress error, please check it!\n");
			ret = -EINVAL;
			break; //address conflict
		}

		inputFile = fopen(fileName,"rb");

		if(NULL == inputFile)
		{
			if(strcmp(fileName,"LUT") == 0)
			{
				printf("\nno lookup table data\n");
				continue;
			}
			else
			{
				printf("\ncan't open %s for read!\n",fileName);
				break;
			}
		}
        printf("processing file %s: OK\n", fileName);

		ret = read_and_write_file(inputFile,outputFile,pInputBuff);
		if((ret < 0) || (currentAddress != endAddress))
		{
			printf("\nread write file error!\n");
			ret = -EINVAL;
			break;
		}

		fclose(inputFile);
		inputFile = NULL;
	}

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

	//check
	if(ret == 0)
	{
        /*if(check_binary(output_file_path) != true)
        {
            ret = -1;
        }*/
    }
	return ret;
}
#endif
int do_merge(int argc, const char *argv)
{
	FILE* inputFile = NULL;
	FILE* outputFile = NULL;
	FILE* cmdFile = NULL;
	unsigned char * pInputBuff =NULL;
	int ret =0 ;
	char fileName[FILE_NAME_SIZE];
	unsigned int startAddress = 0,endAddress = 0;
	Flash_Memofy_Map_T flash_memory_map[FLASH_SECTION_MAX_NUMBER];
	unsigned int section_number = 0;
	unsigned int section_id = 0;

	unsigned int nVersion = FLASH_STRUCTRUE_REVISION;
	unsigned int nCodeExistPattern = CEP;
	unsigned int nLen = 0;
	const char* output_file_path = "bev_nor_flash.bin";
	currentAddress = 0;

	printf("start!\n");

	/*const char* working_dir = "/home/kezhh/Work/Merged_Image";

	if(chdir(working_dir))
	{
        printf("enter %s failed[%u].\n", working_dir, errno);
	}*/

	pInputBuff = (unsigned char*) malloc(BUFFER_SIZE);
	if(NULL == pInputBuff)
	{
		printf("Malloc buffer failed!\n");
		ret = -EINVAL;
		goto EXIT;
	}

	outputFile = fopen(output_file_path,"wb");
	if(NULL == outputFile)
	{
		printf("can't open %s for write!\n", output_file_path);
		ret = -EINVAL;
		goto EXIT;
	}

	cmdFile = fopen("config.txt","r");
	if(NULL == outputFile)
	{
		printf("can't open command.txt for read!\n");
		ret = -EINVAL;
		goto EXIT;
	}

	while(1)
	{
		fscanf(cmdFile,"%s",fileName);
		if(strcmp(fileName,"finish") == 0)
		{
			unsigned int nCrc = 0;
			unsigned int nWrited = 0;
			//write flash memory map into the end, and reserve RESERVED_SIZE space
			//attach 0xFF as pad
			while(currentAddress < FLASH_MEMORY_MAP_START_ADDRESS)
			{
				fputc(0xFF,outputFile);
				currentAddress++;
			}

			//write flash memory map section number
			nWrited = fwrite(&section_number,1,sizeof(section_number),outputFile);
			if(nWrited != sizeof(section_number))
			{
				printf("\nWritefile error!\n");
				ret = -EINVAL;
				goto EXIT;
			}
			currentAddress += sizeof(section_number);

			//write flash memory map each sections
			nWrited = fwrite(flash_memory_map,1,sizeof(Flash_Memofy_Map_T) * section_number,outputFile);
			currentAddress += (sizeof(Flash_Memofy_Map_T) * section_number);
			if(nWrited != (sizeof(Flash_Memofy_Map_T) * section_number))
			{
				printf("\nWritefile error!\n");
				ret = -EINVAL;
				goto EXIT;
			}

			if(currentAddress > FLASH_MEMORY_MAP_END_ADDRESS)
			{
				printf("\nFlash Memory Map Write error!\n");
				ret = -EINVAL;
			}

			//attach 0xFF as pad
			while(currentAddress < FLASH_MEMORY_MAP_END_ADDRESS - USED_BYTE_NUM_FROM_END)
			{
				fputc(0xFF,outputFile);
				currentAddress++;
			}

			nLen = sizeof(section_number) + sizeof(Flash_Memofy_Map_T) * section_number;

			nCrc = nCodeExistPattern + nLen + nVersion + section_number;
			nCrc = check_sum((unsigned char *)flash_memory_map,sizeof(Flash_Memofy_Map_T) * section_number, nCrc);

			//write CEP, Len, Version, CRC
			fwrite(&nCodeExistPattern,4,1,outputFile);
			currentAddress += 4;
			fwrite(&nLen,4,1,outputFile);
			currentAddress += 4;
			fwrite(&nVersion,4,1,outputFile);
			currentAddress += 4;
			fwrite(&nCrc,4,1,outputFile);
			currentAddress += 4;

			//attach 0xFF as pad
			while(currentAddress < FLASH_SIZE)
			{
				fputc(0xFF,outputFile);
				currentAddress++;
			}

			printf("Finished!\n");
			break; //finished
		}
		printf("File : %-15s ",fileName);

		fscanf(cmdFile,"%x",&section_id);

		fscanf(cmdFile,"%x",&startAddress);
		printf("s:0x%-7x ",startAddress);

		fscanf(cmdFile,"%x",&endAddress);
		printf("e:0x%-7x\n",endAddress);

		flash_memory_map[section_number].nSegmentId = section_id;
		flash_memory_map[section_number].nSegmentStartAddress = startAddress;
		flash_memory_map[section_number].nSegmentEndAddress = endAddress;
		section_number ++;

		if((startAddress != currentAddress) || (endAddress > FLASH_SIZE))
		{
			printf("\naddress error, please check it!\n");
			ret = -EINVAL;
			break; //address conflict
		}

		inputFile = fopen(fileName,"rb");

		if(NULL == inputFile)
		{
			if(strcmp(fileName,"LUT") == 0)
			{
				printf("\nno lookup table data\n");
				continue;
			}
			else
			{
				printf("\ncan't open %s for read!\n",fileName);
				break;
			}
		}
                printf("processing file %s: OK\n", fileName);

		ret = read_and_write_file(inputFile,outputFile,pInputBuff);
		if((ret < 0) || (currentAddress != endAddress))
		{
			printf("\nread write file error!\n");
			ret = -EINVAL;
			break;
		}

		fclose(inputFile);
		inputFile = NULL;
	}

EXIT:
	if(NULL != inputFile)
	{
		fclose(inputFile);
	}

	if(NULL != outputFile)
	{
		fclose(outputFile);
	}

	if(NULL != cmdFile)
	{
		fclose(cmdFile);
	}

	if(NULL != pInputBuff)
	{
		free(pInputBuff);
		pInputBuff = NULL;
	}

	//check
	if(ret == 0)
	{
        /*if(check_binary(output_file_path) != true)
        {
            ret = -1;
        }*/
    }
	return ret;
}

bool check_binary(const char* path)
{
    bool ret = false;
    FILE* fi = fopen(path, "rb");
    if(fi != NULL)
    {
        fseek(fi, 0, SEEK_END);
        size_t file_size = ftell(fi);
        fseek(fi, 0, SEEK_SET);

        uint8_t* buffer = (uint8_t*)malloc(file_size);
        if(buffer)
        {
            if(fread(buffer, file_size, 1, fi) >0)
            {
                check_flash_sections(buffer, file_size);
                ret = true;
                free(buffer);
            }
        }
        fclose(fi);
    }
    return ret;
}
/*
int main1(int argc, char* argv[])
{
    const char* working_dir = "/home/kezhh/Work/Merged_Image";
    //const char* working_dir = "/mnt/dev/SVN/Merged_Image";

	if(chdir(working_dir))
	{
        printf("enter %s failed[%u].\n", working_dir, errno);
	}
	check_binary("bev_nor_flash.bin");
	return 0;
}
*/
