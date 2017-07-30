#ifndef __common_H__
#define __common_H__

#ifndef _MSVC_
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif

#if __WORDSIZE == 32
typedef uint32_t ptr_int ;
#else
typedef uint64_t ptr_int ;
#endif

#include <string>
typedef struct
{
	uint32_t id;
	uint32_t start_pos;
	uint32_t end_pos;
	std::string section_name;
	std::string input_file;
	std::string dst_file;
}SectionItem;

#endif // __common_H__
