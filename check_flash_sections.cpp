/*===========================================================================*\
 * FILE: check_flash_secitons.c
 *===========================================================================
 * Copyright 2008 Delphi Technologies, Inc., All Rights Reserved.
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
//#include "reuse.h"
/*===========================================================================*\
 * Other Header Files
\*===========================================================================*/
//#include <common.h>
#include "check_flash_sections.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <wx/log.h>
/*===========================================================================*\
 * Local Preprocessor #define Constants
\*===========================================================================*/

/*===========================================================================*\
 * Local Preprocessor #define MACROS
\*===========================================================================*/

/*===========================================================================*\
 * Local Type Declarations
\*===========================================================================*/
#define SUB_SMC_SIZE    (uint32_t)2048
/*===========================================================================*\
 * External Object Definitions
\*===========================================================================*/

uint8_t gucSubSMC_Num;
/*===========================================================================*\
 * Local Object Definitions
\*===========================================================================*/

/*===========================================================================*\
 * Local Function Prototypes
\*===========================================================================*/
static uint32_t sum_all_4bytes_aligned(uint8_t * start_address, uint32_t length);


/*===========================================================================*\
 * Local Inline Function Definitions and Function-Like Macros
\*===========================================================================*/

/*===========================================================================*\
 * internal Function Definitions
\*===========================================================================*/
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


#define CHECK_SECTIONS_MSGSIZE (1024)

static char debug_string[CHECK_SECTIONS_MSGSIZE] = {0};

static void log_message(char* fmt, ...)
{
    int pos = strlen(debug_string);
    if(pos < CHECK_SECTIONS_MSGSIZE)
    {
        va_list va;

        va_start(va, fmt);

        vsprintf(debug_string + pos, fmt, va);

        va_end(va);

    }
}

void print_check_section_message()
{
    wxLogMessage("%s\n", debug_string);
}

/*===========================================================================*\
 * external Function Definitions
\*===========================================================================*/
Flash_Checksum_Result_T check_flash_sections(uint8_t* buffer, uint32_t buffer_size)
{
   Flash_Section_End_T  flash_section_end;
   ptr_int nor_flash_base_addr = 0;
   Flash_Memofy_Map_T flash_memory_map[FLASH_SECTION_MAX_NUMBER];
   uint32_t section_number = 0;
   uint32_t offset = 0;
   uint32_t len = 0;
   int32_t  i = 0;
   uint32_t section_start = 0;
   uint32_t section_length = 0;
   uint8_t  data_buffer[READ_DATA_COUNT];
   uint32_t section_checksum = 0;

   //nor_flash_base_addr = (uint32_t)NOR_FLASH_BASE_ADDRESS;
   nor_flash_base_addr = (ptr_int)buffer;

   //read partition information section end
   offset = nor_flash_base_addr + FLASH_MEMORY_MAP_END_ADDRESS - USED_BYTE_NUM_FROM_END;
   len = sizeof(Flash_Section_End_T);
   memcpy(&flash_section_end, (uint8_t*)offset, len);

   //judge whether partition information exist
   if(CEP == flash_section_end.code_exist_patten)
   {
      //
   }
   else
   {
       early_printf("Flash exits patten does not exist!\n");
      return CHECKSUM_PARTITION_CEP_ERROR;
   }


   //read section number
   offset = nor_flash_base_addr + FLASH_MEMORY_MAP_START_ADDRESS;
   len = sizeof(section_number);
   memcpy(&section_number, (uint8_t*)offset, len);

   //read flash memory map
   offset =  nor_flash_base_addr + FLASH_MEMORY_MAP_START_ADDRESS + sizeof(section_number);
   len = sizeof(Flash_Memofy_Map_T) * section_number;
   memcpy(flash_memory_map, (uint8_t*)offset, len);

   //calculate flash memory map checksum
   section_checksum = flash_section_end.code_exist_patten + flash_section_end.length + flash_section_end.version;
   section_checksum += section_number;
   section_checksum += sum_all_4bytes_aligned((uint8_t *)flash_memory_map, len);

   //set the checksum and revision

   //match flash memory map checksum
   if((section_checksum + flash_section_end.crc) != 0)
   {
       early_printf("============333333333333===============\n");
      //return CHECKSUM_CHECK_FAIL;
   } //if. no else

   //judge whether partition information exist
   if(FLASH_STRUCTRUE_REVISION == flash_section_end.version)
   {
      //checksum_rev_fst_p->flash_struct_revision_match  = 1;
   }
   else
   {
       early_printf("=====version error 4444======\n");
      //return CHECKSUM_FLASH_STRUCTRUE_REVISION_ERROR;
   }

   //check each section 's checksum
   for(i=0; i< section_number; i++)
   {
      //read section end
      offset =  nor_flash_base_addr + flash_memory_map[i].nSegmentEndAddress - USED_BYTE_NUM_FROM_END;
      len = sizeof(Flash_Section_End_T);
      memcpy(&flash_section_end, (uint8_t*)offset, len);

      //judge whether section exist
      if(CEP == flash_section_end.code_exist_patten)
      {
         //Set_Cep_Exist(checksum_rev_fst_p, flash_memory_map[i].nSegmentId);
      }
      else
      {  if(LUT_SECTION_ID == flash_memory_map[i].nSegmentId)
         {
            *((uint32_t *)LUT_SDRAM_ADDRESS) = 0;  //clear LUT exist flag
            continue;
         }
         else
         {
            return CHECKSUM_SECTION_CEP_ERROR;
         }
      }

      //calc section checksum
      section_start = flash_memory_map[i].nSegmentStartAddress;
      section_length = flash_section_end.length;
      offset =  nor_flash_base_addr + section_start;
      section_checksum = flash_section_end.code_exist_patten + flash_section_end.length + flash_section_end.version;
      early_printf("Will CheckSection:id[%08X], start[%08X], length[%08X], offset[%08X]\n", flash_memory_map[i].nSegmentId, section_start, section_length, offset);
      if(SMC_SECITON_ID == flash_memory_map[i].nSegmentId)
      {
         //read section data
         #if 0
         edma_mem_copy((uint8_t*)SMC_SDRAM_ADDRESS, (uint8_t*)offset, section_length);

         gucSubSMC_Num = section_length/SUB_SMC_SIZE;

         //calc data checksum
         section_checksum += sum_all_4bytes_aligned((uint8_t*)SMC_SDRAM_ADDRESS, section_length);
         #else
         gucSubSMC_Num = section_length/SUB_SMC_SIZE;
         section_checksum += sum_all_4bytes_aligned((uint8_t*)offset, section_length);
         #endif

         //set the checksum and revision
         //Set_Flash_Checksum(checksum_rev_fst_p, flash_memory_map[i].nSegmentId, flash_section_end.crc);
         //Set_Runtime_Checksum(checksum_rev_sec_p, flash_memory_map[i].nSegmentId, section_checksum);
         //Set_Revision(checksum_rev_sec_p, flash_memory_map[i].nSegmentId, flash_section_end.version);

         //match checksum
         if((section_checksum + flash_section_end.crc) == 0)
         {
            //*((uint32_t *)SMC_SDRAM_ADDRESS) = 0xAA5555AA;  //set SMC exist flag
         }
         else
         {
            //*((uint32_t *)SMC_SDRAM_ADDRESS) = 0;  //clear SMC exist flag
            early_printf("SMC section checksum error\n");
            return CHECKSUM_CHECK_FAIL;
         } //if. no else

         continue;
      }
      else if(LUT_SECTION_ID == flash_memory_map[i].nSegmentId)
      {
         //read section data
         #if 0
         edma_mem_copy((uint8_t*)LUT_SDRAM_ADDRESS, (uint8_t*)offset, section_length);

         //calc data checksum
         section_checksum += sum_all_4bytes_aligned((uint8_t*)LUT_SDRAM_ADDRESS, section_length);
         #else
         section_checksum += sum_all_4bytes_aligned((uint8_t*)offset, section_length);
         #endif
         //set the checksum and revision
         //Set_Flash_Checksum(checksum_rev_fst_p, flash_memory_map[i].nSegmentId, flash_section_end.crc);
         //Set_Runtime_Checksum(checksum_rev_sec_p, flash_memory_map[i].nSegmentId, section_checksum);

         //match LUT checksum
         if((section_checksum + flash_section_end.crc) == 0)
         {
            //*((uint32_t *)LUT_SDRAM_ADDRESS) = 0xAA5555AA;  //set LUT exist flag
            /* copy flash_section_end to DDR */
            //memcpy((void *)(LUT_SDRAM_ADDRESS + flash_memory_map[i].nSegmentEndAddress - flash_memory_map[i].nSegmentStartAddress - USED_BYTE_NUM_FROM_END), &flash_section_end, sizeof(Flash_Section_End_T));
         }
         else
         {
            //*((uint32_t *)LUT_SDRAM_ADDRESS) = 0;  //clear LUT exist flag
         }//if. no else
         continue;
      }
      else
      {
        /*section_checksum = sum_all_4bytes_aligned(offset, section_length);
        section_checksum = 0;*/
         while(section_length > 0)
         {
            //read section data
            len = (section_length > READ_DATA_COUNT ? READ_DATA_COUNT : section_length);
            memcpy(data_buffer, (uint8_t*)offset, len);

            //calc data checksum
            section_checksum += sum_all_4bytes_aligned(data_buffer, len);
            section_length -= len;
            offset += len;
         } //while
      }

      //set the checksum and revision
//      Set_Flash_Checksum(checksum_rev_fst_p, flash_memory_map[i].nSegmentId, flash_section_end.crc);
      //Set_Runtime_Checksum(checksum_rev_sec_p, flash_memory_map[i].nSegmentId, section_checksum);
      //Set_Revision(checksum_rev_sec_p, flash_memory_map[i].nSegmentId, flash_section_end.version);

      //match checksum
      if((section_checksum + flash_section_end.crc) != 0)
      {
        early_printf("Section [%d:%08X] checksum not match: cep:%08X,length:%08X,version:%08X,crc:%08X.\n", i, flash_memory_map[i].nSegmentId,
            flash_section_end.code_exist_patten, flash_section_end.length, flash_section_end.version, flash_section_end.crc);
         return CHECKSUM_CHECK_FAIL;
      } //if. no else
   } //for

   return CHECKSUM_CHECK_PASS;
}

void LoadSubSMC(uint8_t ucSMCIndex)
{
     if(0 < ucSMCIndex)
     {
         //edma_mem_copy(SMC_SDRAM_ADDRESS, (SMC_SDRAM_ADDRESS + ucSMCIndex*SUB_SMC_SIZE), SUB_SMC_SIZE);
     }
}

/*===========================================================================*\
 * File Revision History (top to bottom: first revision to last revision)
 *===========================================================================
 *
 *   Date        userid       Description
 * ----------- ----------    -----------
 * 12/04/14     Bao Shang    Inital version
 * 01/15/15     Bao Shang    Add transfter to IP module
 * 08/10/2015 Wenxin Fang Add Multi-SMC
\*===========================================================================*/

