#ifndef CHECK_FLASH_SECTIONS_H
#define CHECK_FLASH_SECTIONS_H
/*===========================================================================*\
 * FILE: check_flash_sections.h
 *===========================================================================
 * Copyright 2003 Delphi Technologies, Inc., All Rights Reserved.
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
#include "common.h"
/*===========================================================================*\
 * Other Header Files
\*===========================================================================*/
//#include "com_with_mcu.h"
/*===========================================================================*\
 * Exported Preprocessor #define Constants
\*===========================================================================*/

#ifdef __cplusplus
extern "C"
{
#endif
/*===========================================================================*\
 * Exported Preprocessor #define MACROS
\*===========================================================================*/
#define USED_BYTE_NUM_FROM_END ((uint32_t)16)
#define FLASH_SECTION_MAX_NUMBER ((uint32_t)0x20)
#define READ_DATA_COUNT ((uint32_t)1024)
#define NOR_FLASH_BASE_ADDRESS ((uint32_t)0x08000000)

#define USC_SPACE_START     ((uint32_t)0x1400000)  //USC be stored here
#define USC_SPACE_END       ((uint32_t)0x1500000)
#define LUT_SPACE_START     ((uint32_t)0x1500000)  //LUT be stored here
#define LUT_SPACE_END       ((uint32_t)0x1B00000)
#define FLASH_MEMORY_MAP_START_ADDRESS    ((uint32_t)0x1FFF800) //1k size
#define FLASH_MEMORY_MAP_END_ADDRESS      ((uint32_t)0x1FFFC00)

#define LUT_SDRAM_ADDRESS         ((uint32_t)0x85300000)    //LUT will be load to this address
#define SMC_SDRAM_ADDRESS         ((uint32_t)0x85C00000)    //USC will be load to this address

#define FLASH_SECTION_END_SIZE ((uint32_t)16)
#define CEP ((uint32_t)0x55AA55AA)
/*===========================================================================*\
 * Exported Type Declarations
\*===========================================================================*/

#define FLASH_STRUCTRUE_REVISION (unsigned int)0x00010000
#define early_printf printf
#define edma_mem_copy memcpy
typedef struct Flash_Memory_Map_Tag
{
   uint32_t nSegmentId;
   uint32_t nSegmentStartAddress;
   uint32_t nSegmentEndAddress;
} Flash_Memofy_Map_T;

typedef struct Flash_Section_Name_Tag
{
   uint32_t nSegmentId;
   char     strSectionName[20];
} Flash_Section_Name_T;

typedef struct Flash_Section_End_Tag
{
   uint32_t code_exist_patten;
   uint32_t length;
   uint32_t version;
   uint32_t crc;
} Flash_Section_End_T;

typedef enum Flash_Checksum_Verify_Tag
{
   CHECKSUM_CHECK_PASS = 0,
   CHECKSUM_CHECK_FAIL = 1,
   CHECKSUM_CHECK_NOT_FINISH = 2,
   CHECKSUM_PARTITION_CEP_ERROR = 3,
   CHECKSUM_SECTION_NUMBER_ERROR = 4,
   CHECKSUM_SECTION_CEP_ERROR = 5,
   CHECKSUM_FLASH_STRUCTRUE_REVISION_ERROR = 6,
} Flash_Checksum_Result_T;

typedef enum Flash_Section_ID_Tag
{
   FIRST_BOOT_SECITON_ID           = 0x55AA0000,
   KERNEL_SECITON_ID               = 0x55AA0001,
   FILE_SYSTEM_SECITON_ID          = 0x55AA0002,
   DSP_PROGRAM_SECITON_ID          = 0x55AA0003,
   M3_PROGRAM_SECITON_ID           = 0x55AA0004,
   SMC_SECITON_ID                  = 0x55AA0005,
   LUT_SECTION_ID                  = 0x55AA0006,
   FLASH_MEMORY_MAP_SECITON_ID     = 0x55AA0099
} Flash_Section_ID_T;

/*===========================================================================*\
 * Exported Object Declarations
\*===========================================================================*/

/*===========================================================================*\
 * Exported Function
\*===========================================================================*/
Flash_Checksum_Result_T check_flash_sections(uint8_t* buffer, uint32_t buffer_size);
/*===========================================================================*\
 * Exported Inline Function Definitions and #define Function-Like Macros
\*========================Prototypes===================================================*/

/*===========================================================================*\
 * File Revision History (top to bottom: first revision to last revision)
 *===========================================================================
 *
 * Date        userid    (Description on following lines: SCR #, etc.)
 * ----------- --------
 * 12/04/14  Bao Shang     Inital version
 * 01/15/15  Bao Shang     Add transfter to IP module
\*===========================================================================*/
#ifdef __cplusplus
};
#endif

#endif //CHECK_FLASH_SECTIONS_H

