#ifndef __merge_H__
#define __merge_H__

#include <wx/wx.h>
#include "common.h"
#include <vector>

#define NOCONFIG_FILE_SUPPORT

#ifdef NOCONFIG_FILE_SUPPORT
int do_merge(std::vector<SectionItem>& sections, const char* output_file);
#endif
int do_merge(int argc, const char* argv);
bool check_binary(const char* path);

#endif
