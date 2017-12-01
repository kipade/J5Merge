/***************************************************************
 * Name:      J5MergeMain.cpp
 * Purpose:   Code for Application Frame
 * Author:    知遇 (178041876@qq.com)
 * Created:   2017-02-14
 * Copyright: 知遇 ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#include "merge.h"

#include <wx/xrc/xmlres.h>
#include "file_checksum.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "J5MergeMain.h"
#include "common.h"
#include <wx/process.h>
#include <wx/dir.h>
#include <wx/arrstr.h>
#include <wx/regex.h>

#include <algorithm>

#include "CramfsFile.h"

//helper functions
enum wxbuildinfoformat {
    short_f, long_f };

wxString wxbuildinfo(wxbuildinfoformat format)
{
    wxString wxbuild(wxVERSION_STRING);

    if (format == long_f )
    {
#if defined(__WXMSW__)
        wxbuild << _T("-Windows");
#elif defined(__WXMAC__)
        wxbuild << _T("-Mac");
#elif defined(__UNIX__)
        wxbuild << _T("-Linux");
#endif

#if wxUSE_UNICODE
        wxbuild << _T("-Unicode build");
#else
        wxbuild << _T("-ANSI build");
#endif // wxUSE_UNICODE
    }

    return wxbuild;
}

BEGIN_EVENT_TABLE(J5MergeFrame, wxFrame)
    EVT_CLOSE(J5MergeFrame::OnClose)
    EVT_MENU(idMenuQuit, J5MergeFrame::OnQuit)
    EVT_MENU(idMenuAbout, J5MergeFrame::OnAbout)
    EVT_MENU(idMenuLoadDir, J5MergeFrame::OnMenuLoadFromDir)
    EVT_MENU(idMenuShowLog, J5MergeFrame::OnMenuShowLog)
    EVT_MENU(idMenuClearLog, J5MergeFrame::OnMenuClearLog)
    //EVT_BUTTON(wxID_ANY, J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(wxID_OK, J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(wxID_CANCEL, J5MergeFrame::OnAnyButtonPressed)

    EVT_BUTTON(XRCID("btn-browse-uboot"), J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(XRCID("btn-browse-dsp"), J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(XRCID("btn-browse-m3"), J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(XRCID("btn-browse-uimage"), J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(XRCID("btn-browse-fs"), J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(XRCID("btn-browse-smc"), J5MergeFrame::OnAnyButtonPressed)
    EVT_BUTTON(XRCID("btn-browse-lut"), J5MergeFrame::OnAnyButtonPressed)

    EVT_BUTTON(XRCID("btn_deal_uboot"), J5MergeFrame::OnGenerateComponent)
    EVT_BUTTON(XRCID("btn_deal_m3"), J5MergeFrame::OnGenerateComponent)
    EVT_BUTTON(XRCID("btn_deal_dsp"), J5MergeFrame::OnGenerateComponent)
    EVT_BUTTON(XRCID("btn_deal_uimage"), J5MergeFrame::OnGenerateComponent)
    EVT_BUTTON(XRCID("btn_deal_fs"), J5MergeFrame::OnGenerateComponent)
    EVT_BUTTON(XRCID("btn_deal_smc"), J5MergeFrame::OnGenerateComponent)
    EVT_BUTTON(XRCID("btn_deal_lut"), J5MergeFrame::OnGenerateComponent)
END_EVENT_TABLE()

J5MergeFrame::J5MergeFrame(wxFrame *frame, const wxString& title) : recent_source_dir(wxEmptyString), cfg_path("j5merge.cfg")
//    : wxFrame(frame, -1, title)
{
    //wxCommandEventHandler(J5MergeFrame::OnGenerateComponent);
    // wxEventTableEntry(0, 0, 0, wxNewEventTableFunctor(0, &J5MergeFrame::OnGenerateComponent), NULL);
    //wxEventFunctor* f =
    if(wxXmlResource().Get()->LoadFrame(this, NULL, wxT("main_frame")) != true)
    {
        wxMessageBox(wxT("加载窗口资源失败!"));
        exit(-1);
    }
    SelectMergeConfig();
#if wxUSE_MENUS
    // create a menu bar
    wxMenuBar* mbar = new wxMenuBar();
    wxMenu* fileMenu = new wxMenu(_T(""));
    fileMenu->Append(idMenuQuit, wxT("退出\tAlt-F4"), wxT("退出程序"));
    fileMenu->Append(idMenuLoadDir, wxT("从目录加载..."), wxT("从指定目录搜索可能的源文件"));
    mbar->Append(fileMenu, wxT("文件"));

    wxMenu* helpMenu = new wxMenu(wxT(""));
    wxMenu* logMenu = new wxMenu(wxT(""));
    logMenu->AppendCheckItem(idMenuShowLog, wxT("显示日志"));
    logMenu->Append(idMenuClearLog, wxT("清空日志"));
    helpMenu->AppendSubMenu(logMenu, wxT("日志"));
    helpMenu->Append(idMenuAbout, wxT("关于\tF1"), wxT("关于本程序"));
    mbar->Append(helpMenu, wxT("帮助"));

    SetMenuBar(mbar);
#endif // wxUSE_MENUS

#if wxUSE_STATUSBAR
    // create a status bar with some information about the used wxWidgets version
    CreateStatusBar(2);
    SetStatusText(wxT("欢迎使用!"),0);
    SetStatusText(wxT("J5MergeApp v0.5"), 1);
#endif // wxUSE_STATUSBAR
    InitControls();
    wxSizer* topSizer = GetSizer();
    topSizer->Fit(this);
    ParseConfig();

    /*
    config_t cfg;
    config_init(&cfg);

    if(config_read_file(&cfg, cfg_path) == CONFIG_TRUE)
    {
        const char* recent_src_dir = NULL;
        if(config_lookup_string(&cfg, "last_source_dir", &recent_src_dir) == CONFIG_TRUE)
        {
            wxString dir_name(recent_src_dir);
            if(wxDirExists(dir_name))
            {
                recent_source_dir = dir_name;
            }
        }
        if(config_lookup_string(&cfg, "last_autoload_dir", &recent_src_dir) == CONFIG_TRUE)
        {
            wxString dir_name(recent_src_dir);
            if(wxDirExists(dir_name))
            {
                recent_autoload_dir = dir_name;
            }
        }
        config_destroy(&cfg);
    }*/
    LoadAppConfigs();

}


J5MergeFrame::~J5MergeFrame()
{
}

void J5MergeFrame::OnClose(wxCloseEvent &event)
{
    SaveAppConfigs();
    Destroy();
}

void J5MergeFrame::OnQuit(wxCommandEvent &event)
{
    Destroy();
}

void J5MergeFrame::OnAbout(wxCommandEvent &event)
{
    //wxString msg = wxbuildinfo(long_f);
    wxMessageBox(wxT("J5MergeApp v0.5"), wxT("关于..."));
}

static void ReplaceZeroWithSpace(char* buffer, int len)
{
    for(int i = 0; i < len; ++i)
    {
        if(buffer[i] == 0)
            buffer[i] = ' ';
    }
}

static wxString GetFileVersionByPos(const wxString& path, uint32_t pos, bool little_endian = true)
{
    wxFile fi(path, wxFile::read);
    if(fi.IsOpened())
    {
        uint32_t sz = fi.Length();
        if(sz >= (pos + 4))
        {
            fi.Seek(pos);
            uint8_t version[4];
            *(uint32_t*)version = 0;
            fi.Read(version, sizeof(version));

            if(little_endian)
            {
                for(int i = 3; i >= 0; --i)
                {
                    if(version[i] == 0)
                        version[i] = 0xFF;
                    else
                        break;
                }
                return wxString::Format(wxT("%02X.%02X.%02X.%02X"), version[3], version[2], version[1], version[0]);
            }else
            {
                for(int i = 0; i < 4; ++i)
                {
                    if(version[i] == 0)
                        version[i] = 0xFF;
                    else
                        break;
                }
                return wxString::Format(wxT("%02X.%02X.%02X.%02X"), version[0], version[1], version[2], version[3]);
            }
        }
    }
    return wxT("FF.FF.FF.FF");
}

static wxString GetFileVersionByTag(const wxString& filepath, const char* tag)
{
#if 1//ifdef linux
    //const char* tag = "U-Boot v";
    wxString cmd = wxString::Format(wxT("grep -a \"%s\" %s"), tag, filepath);

    wxProcess process;
    process.Redirect();
    const char* version = NULL;
    long ret = wxExecute(cmd, wxEXEC_SYNC, &process);
    if(ret >= 0)
    {
        wxInputStream* stm = process.GetInputStream();
        char buffer[1024] = {0};
        buffer[1023] = 0;
        if(stm->CanRead())
        {

            do
            {
                stm->Read(buffer, 1023);
                ReplaceZeroWithSpace(buffer, 1023);
                version = strstr(buffer, tag);
                if(version != NULL)
                {
                    break;
                }
            }while(stm->Eof() == false);


            if(version != NULL)
            {
                version += strlen(tag);
                wxString strVersion;
                const char* p = version;
                while(*p != ' ' && *p != 0)
                {
                    strVersion.Append(*p);
                    p ++;
                }
                return strVersion;
            }//end if version!= NULL
        }//end if stm->CanRead

    }//end if ret>=0

#else

#endif
    return wxString("FF.FF.FF.FF");
}

static wxString GetVersionTag(uint8_t* buffer, size_t size, const char* tag)
{
    size_t tag_len = strlen(tag);
    uint8_t* pos = buffer;
    
    while(pos < (buffer + (size - tag_len)))
    {
        if(strncmp((const char*)pos, tag, tag_len) == 0)
        {
            const char* version_str = (const char*)pos;
            if(strlen(version_str + tag_len) > 5)
            {
                return wxString(version_str + tag_len);
            }
        }
        pos++;
    }
    return wxT("FF.FF.FF.FF");
}

static wxString GetCramfsApphostVersion(const wxString& path)
{
    wxString version = wxT("FF.FF.FF.FF");
    CramfsFile cramfs(path.c_str());
    uint8_t* file_buffer;
    size_t file_size;
    if(cramfs.ExtractFile("/opt/app_host", file_buffer, file_size))
    {
        version = GetVersionTag(file_buffer, file_size, "version(str):");
        version.Strip();
        cramfs.FreeExtracedFileBuffer(file_buffer, file_size);
    }
    return version;
}

static wxString GetJ5FileVersion(uint32_t comp_id, const wxString& path)
{
    switch(comp_id)
    {
    case 0x55AA0000:
        return GetFileVersionByTag(path, "U-Boot v");
    case 0x55AA0002://文件系统
        return GetCramfsApphostVersion(path);
    case 0x55AA0003:
    case 0x55AA0004:
        return GetFileVersionByTag(path, "version:");
    case 0x55AA0005:
        return GetFileVersionByPos(path, 0, false);
    case 0x55AA0006:
        return GetFileVersionByPos(path, 0);
    }
    return wxString("FF.FF.FF.FF");
}
/*
static bool GenerateFinalFile(const wxString& src, const wxString& dst, uint32_t output_size, const wxString& version)
{
    //if(do_gen_checked_file(src.ToAscii()))
    char str_outputsize[16];
    sprintf(str_outputsize, "%d", output_size);

    const char* args[5];
    args[0] = src.mb_str();
    args[1] = src.mb_str();
    args[2] = dst.mb_str();
    args[3] = str_outputsize;
    args[4] = version.mb_str();

    if(do_gen_checked_file(5, (char**)args) == 0)
        return true;

    return false;
}
*/
bool J5MergeFrame::SelectMergeConfig()
{
    wxFileDialog dlg(this, wxT("选择合成配置文件..."), wxT("configs"), wxEmptyString, wxT("配置文件 (*.cfg)|*.cfg| 所有文件 (*.*)|*.*"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if(dlg.ShowModal() == wxID_OK)
    {
        const wxString path = dlg.GetPath();
        LoadMergeConfig(path);
        return true;
    }
    return false;
}

bool J5MergeFrame::LoadMergeConfig(const wxString& path)
{
    FILE* fi = fopen(path.c_str(), "rt");
    if(fi != NULL)
    {
        Config cfg;
        cfg.read(fi);
        ParseMergeConfig(cfg);
        SortMergeConfigByOffset();
        fclose(fi);
        return true;
    }
    return false;
}

int J5MergeFrame::ParseMergeConfig(const Config& merge_cfg)
{
    try
    {
        merge_configs.clear();
        Setting& root = merge_cfg.lookup("sections_info");
        Setting::Type type = root.getType();
        if(type == Setting::TypeGroup)
        {
            int items_count = root.getLength();
            for(int i = 0; i < items_count; ++i)
            {
                Setting& cfg = root[i];
                if(cfg.exists("id"))
                {
                    MergeConfigItem item;
                    memset(&item, 0, sizeof(item));
                    cfg.lookupValue("id", item.id);

                    const char* name = cfg.getName();
                    item.name = strdup(name);

                    if(cfg.exists("offset"))
                    {
                        cfg.lookupValue("offset", item.offset);
                    }
                    if(cfg.exists("length"))
                    {
                        cfg.lookupValue("length", item.length);
                    }
                    item.end = item.offset + item.length;
                    merge_configs.push_back(item);

                }
            }
        }
    }catch(...)
    {
    }
    return merge_configs.size();
}

bool J5MergeFrame::CmpMergeCfgItem(const MergeConfigItem& it1, const MergeConfigItem& it2)
{
    return it1.offset < it2.offset;
}

void J5MergeFrame::SortMergeConfigByOffset()
{
    std::sort(merge_configs.begin(), merge_configs.end(), J5MergeFrame::CmpMergeCfgItem);
}

void J5MergeFrame::OnAnyButtonPressed(wxCommandEvent& event)
{
    int id = event.GetId();
    if(event.GetId() == wxID_OK)
    {
        wxString dst_name("bev_nor_flash.bin");
        if(DoMerge(dst_name))
        {
            SetStatusText(wxT("合成成功！"),0);
            wxMessageBox(wxT("生成成功!"));
        }
        else
            SetStatusText(wxT("合成失败！"),0);
        DeleteAllSectionFiles();//无论如何均清理临时文件
        return;
    }else if(id == wxID_CANCEL)
    {
        Close();
        return;
    }


    wxTextCtrl* text_src_path = NULL;
    wxTextCtrl* text_version = NULL;
    wxString wild_str = wxT("bin 文件 (*.bin)|*.bin|所有文件 (*.*)|*.*");
    wxString default_dir = recent_source_dir;
    uint32_t comp_id;

    if(XRCID("btn-browse-uboot") == id)
    {
        text_src_path = text_uboot_src_path;
        text_version = text_uboot_version;
        comp_id = 0x55AA0000;
    }else if(XRCID("btn-browse-dsp") == id)
    {
        text_src_path = text_dsp_src_path;
        text_version = text_dsp_version;
        comp_id = 0x55AA0003;
        wild_str = wxT("dsp 文件 (*.xe674)|*.xe674|所有文件 (*.*)|*.*");
    }else if(XRCID("btn-browse-m3") == id)
    {
        text_src_path = text_m3_src_path;
        text_version = text_m3_version;
        comp_id = 0x55AA0004;
        wild_str = wxT("M3 文件 (*.xem3)|*.xem3|所有文件 (*.*)|*.*");
    }else if(XRCID("btn-browse-uimage") == id)
    {
        text_src_path = text_uimage_src_path;
        text_version = text_uimage_version;
        comp_id = 0x55AA0001;
        wild_str = wxT("所有文件 (*)|*");
    }else if(XRCID("btn-browse-fs") == id)
    {
        text_src_path = text_fs_src_path;
        text_version = text_fs_version;
        comp_id = 0x55AA0002;
    }else if(XRCID("btn-browse-smc") == id)
    {
        text_src_path = text_smc_src_path;
        text_version = text_smc_version;
        comp_id = 0x55AA0005;
    }else if(XRCID("btn-browse-lut") == id)
    {
        text_src_path = text_lut_src_path;
        text_version = text_lut_version;
        comp_id = 0x55AA0006;
    }
    wxFileDialog dlg(this, wxT("选择源文件..."), default_dir, wxEmptyString, wild_str, wxFD_OPEN|wxFD_FILE_MUST_EXIST);
    if(text_src_path != NULL && text_version && dlg.ShowModal() == wxID_OK)
    {
        wxString src_path = dlg.GetPath();
        wxString src_file = dlg.GetFilename();
        text_src_path->SetValue(src_path);//此时文件必定存在
        wxString file_version = GetJ5FileVersion(comp_id, src_path);
        text_version->SetValue(file_version);
        recent_source_dir = dlg.GetDirectory();
        
        if(comp_id == 0x55AA0002)
        {//文件系统
            if(!text_uimage_src_path->GetValue().empty())
            {//如果uimage路径非空，则取版本
                text_uimage_version->SetValue(file_version);
            }
        }else if(comp_id == 0x55AA0001)
        {
            if(file_version.Cmp(wxT("FF.FF.FF.FF")) == 0)//如果uImage版本是有效版本，并且文件系统版本为空，则用它填充
            {
                wxString fs_version = text_fs_version->GetValue();
                if(fs_version.Length() != 0)
                {
                    text_uimage_version->SetValue(fs_version);
                }
            }
        }
    }

}

bool J5MergeFrame::InitControls()
{
    text_uboot_src_path = XRCCTRL(*this, "text-u-boot-src-path", wxTextCtrl);
    text_dsp_src_path = XRCCTRL(*this, "text-dsp-src-path", wxTextCtrl);
    text_m3_src_path = XRCCTRL(*this, "text-m3-src-path", wxTextCtrl);
    text_uimage_src_path = XRCCTRL(*this, "text-uimage-src-path", wxTextCtrl);
    text_fs_src_path = XRCCTRL(*this, "text-fs-src-path", wxTextCtrl);
    text_smc_src_path = XRCCTRL(*this, "text-smc-src-path", wxTextCtrl);
    text_lut_src_path = XRCCTRL(*this, "text-lut-src-path", wxTextCtrl);

    text_uboot_version = XRCCTRL(*this, "text-uboot-version", wxTextCtrl);
    text_dsp_version = XRCCTRL(*this, "text-dsp-version", wxTextCtrl);
    text_m3_version = XRCCTRL(*this, "text-m3-version", wxTextCtrl);
    text_uimage_version = XRCCTRL(*this, "text-uimage-version", wxTextCtrl);
    text_fs_version = XRCCTRL(*this, "text-fs-version", wxTextCtrl);
    text_smc_version = XRCCTRL(*this, "text-smc-version", wxTextCtrl);
    text_lut_version = XRCCTRL(*this, "text-lut-version", wxTextCtrl);

    text_runlog = XRCCTRL(*this, "text_runlog", wxTextCtrl);
    if(text_runlog != NULL)
    {
        wxLog::SetActiveTarget(new wxLogTextCtrl(text_runlog));

        wxSizerItem* log_sizer = GetSizer()->GetItemById(XRCID("log_sizeritem"), true);
        if(log_sizer != NULL)
        {
            log_sizer->Show(false);
        }
    }

    return true;
}

bool J5MergeFrame::DoMerge(const wxString& dst)
{
    std::vector<SectionItem> target_sections;
    if(GenerateCheckedFiles(&target_sections))
    {
        #ifdef NOCONFIG_FILE_SUPPORT
        if(do_merge(target_sections, NULL) == 0)
        #else
        if(do_merge(1, args[0]) ==0 )
        #endif
        {
            std::string val;
            const char* setting_path = "last_autoload_dir";
            if(config.lookupValue(setting_path, val))
            {
                val = recent_source_dir.mbc_str();
                Setting& item = config.lookup(setting_path);

                item = val;
            }
            return true;
        }
    }
    return false;
}


bool J5MergeFrame::GenerateCheckedFiles(std::vector<SectionItem>* sections)
{
    wxTextCtrl* texts[] = {text_uboot_src_path,text_dsp_src_path,text_m3_src_path,text_uimage_src_path,text_fs_src_path, text_smc_src_path};
    bool temp_sections = false;

    for(size_t i = 0; i < sizeof(texts)/sizeof(wxTextCtrl*); ++i)
    {
        wxString label = texts[i]->GetValue();
        if(label.IsEmpty())
        {
            wxMessageBox(wxT("请选择必需的源文件"));
            return false;
        }
    }
    if(sections == NULL)
    {
        sections = new std::vector<SectionItem>;
        temp_sections = true;
    }

    wxString src_path = text_uboot_src_path->GetValue();
    wxString src_version = text_uboot_version->GetValue();
    if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0000, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成uboot中间文件失败"));
        return false;
    }

    sections->push_back(*GetSection(0x55AA0000));

    src_path = text_uimage_src_path->GetValue();
    src_version = text_uimage_version->GetValue();
    if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0001, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成uImage中间文件失败"));
        return false;
    }
    sections->push_back(*GetSection(0x55AA0001));

    src_path = text_fs_src_path->GetValue();
    src_version = text_fs_version->GetValue();
    if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0002, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成Filesystem中间文件失败"));
        return false;
    }
    sections->push_back(*GetSection(0x55AA0002));

    src_path = text_dsp_src_path->GetValue();
    src_version = text_dsp_version->GetValue();
    if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0003, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成DSP中间文件失败"));
        return false;
    }
    sections->push_back(*GetSection(0x55AA0003));

    src_path = text_m3_src_path->GetValue();
    src_version = text_m3_version->GetValue();
    if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0004, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成M3中间文件失败"));
        return false;
    }
    sections->push_back(*GetSection(0x55AA0004));

    src_path = text_smc_src_path->GetValue();
    src_version = text_smc_version->GetValue();
    if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0005, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成SMC中间文件失败"));
        return false;
    }
    sections->push_back(*GetSection(0x55AA0005));

    src_path = text_lut_src_path->GetValue();
    if(src_path.IsEmpty() == false)
    {
        src_version = text_lut_version->GetValue();
        if(GenerateCheckedFile(src_path, "First_boot", 0x55AA0006, wxString("v") + src_version) == false)
        {
            wxMessageBox(wxT("生成LUT中间文件失败"));
            return false;
        }
    }
    sections->push_back(*GetSection(0x55AA0006));
    if(temp_sections)
    {
        delete sections;
        sections = NULL;
    }
    return true;
}

wxString J5MergeFrame::PreprocessDspM3(const wxString& src)
{
    wxString output_file("tmp");
#ifdef WIN32
    wxString cmd = wxString::Format(wxT("out2rprc.exe %s %s"), src, output_file);
#else
    wxString cmd = wxString::Format(wxT("wine out2rprc.exe %s %s"), src, output_file);
#endif
    if(wxExecute(cmd, wxEXEC_SYNC) == 0)
    {
        if(wxFileName::Exists(output_file))
        {
            return output_file;
        }
    }
    return wxEmptyString;
}

bool J5MergeFrame::GenerateCheckedFile(const wxString& src, const wxString& dst, uint32_t id, const wxString& version)
{
    SectionItem *section = GetSection(id);
    if(section != NULL)
    {
        char buffer[32];
        uint32_t section_size = section->end_pos - section->start_pos;
        const char* args[5];
        wxString input = src;

        wxString status_msg = wxString::Format(wxT("%s 正在生成..."), section->dst_file);
        SetStatusText(status_msg,0);

        if(id == 0x55AA0003 || id == 0x55AA0004)
        {
            input = PreprocessDspM3(src);
            if(input.Length() <= 0)
                return false;
        }


        sprintf(buffer, "%x", section_size);
        args[0] = src.mb_str();
        args[1] = input.mb_str();
        args[2] = section->dst_file.c_str();//.mb_str();
        args[3] = buffer;
        args[4] = version.mb_str();
        bool ret = false;
        if(do_gen_checked_file(5, (char**)args) == 0)
        {
            ret = true;
        }
        if(input.Cmp(src))
        {
            wxRemoveFile(input);
        }
        status_msg = wxString::Format(wxT("%s 生成[%s]"), section->dst_file, ret==true?wxT("成功"):wxT("失败"));
        SetStatusText(status_msg,0);
        return ret;
    }
    return false;
}

bool J5MergeFrame::ParseConfig()
{
    for(size_t i = 0; i < merge_configs.size(); ++i)
    {
        SectionItem item;
        item.id = merge_configs[i].id;
        item.section_name = merge_configs[i].name;
        item.start_pos = merge_configs[i].offset;
        item.end_pos = merge_configs[i].end;
        item.dst_file = merge_configs[i].name;
        section_items.push_back(item);
    }
    if(section_items.size() <= 0)
    {
        wxMessageBox(wxT("未找到合适的合成配置选项，可能无法正确合成!"));
        return false;
    }
    return true;

}

SectionItem* J5MergeFrame::GetSection(uint32_t id)
{

    for(size_t i=0 ; i < section_items.size(); ++i)
    {
        if(section_items[i].id == id)
        {
            return &section_items[i];
        }
    }
    return NULL;
}

void J5MergeFrame::DeleteAllSectionFiles()
{
    for(size_t i = 0; i < section_items.size(); ++i)
    {
        if(wxFileExists(section_items[i].dst_file))
            wxRemoveFile(section_items[i].dst_file);
    }
}

void J5MergeFrame::OnGenerateComponent(wxCommandEvent& event)
{
    int id = event.GetId();

    wxTextCtrl* txt_src = NULL;
    wxString src_version;
    wxString src_path;
    uint32_t item_id;

    if(id == XRCID("btn_deal_uboot"))
    {
        txt_src = text_uboot_src_path;
        item_id = 0x55AA0000;
    }else if(id == XRCID("btn_deal_uimage"))
    {
        txt_src = text_uimage_src_path;
        item_id = 0x55AA0001;
    }else if(id == XRCID("btn_deal_fs"))
    {
        txt_src = text_fs_src_path;
        item_id = 0x55AA0002;
    }else if(id == XRCID("btn_deal_smc"))
    {
        txt_src = text_smc_src_path;
        item_id = 0x55AA0005;
    }else if(id == XRCID("btn_deal_lut"))
    {
        txt_src = text_lut_src_path;
        item_id = 0x55AA0006;
    }else if(id == XRCID("btn_deal_dsp"))
    {
        txt_src = text_dsp_src_path;
        item_id = 0x55AA0003;
    }else if(id == XRCID("btn_deal_m3"))
    {
        txt_src = text_m3_src_path;
        item_id = 0x55AA0004;
    }

    if(txt_src != NULL)
    {
        src_path = txt_src->GetValue();
        src_version = text_uboot_version->GetValue();
    }

    if(src_path.IsEmpty())
        return;

    if(GenerateCheckedFile(src_path, "First_boot", item_id, wxString("v") + src_version) == false)
    {
        wxMessageBox(wxT("生成中间文件失败!"));
    }
    else
    {
        wxMessageBox(wxT("生成中间文件成功!"));
    }
}

void J5MergeFrame::OnMenuLoadFromDir(wxCommandEvent& event)
{
    wxDirDialog dlg(this, wxT("选择释放目录"), recent_autoload_dir);
    if(dlg.ShowModal() == wxID_OK)
    {
        RetsetControls();
        recent_autoload_dir = dlg.GetPath();
        LoadSrcFilesFromDir(recent_autoload_dir);
    }
}

class MyTraverser : public wxDirTraverser
{
public:
    MyTraverser(wxArrayString& files) : m_files(files) { }

    virtual wxDirTraverseResult OnFile(const wxString& filename)// wxOVERRIDE
    {
        m_files.push_back(filename);
        return wxDIR_CONTINUE;
    }

    virtual wxDirTraverseResult OnDir(const wxString& dirname)// wxOVERRIDE
    {
        m_files.push_back(dirname);
        return wxDIR_CONTINUE;
    }

private:
    wxArrayString& m_files;

    wxDECLARE_NO_COPY_CLASS(MyTraverser);
};


static wxString SearchSpecifiedFile(const wxString& path, const wxString& filespec)
{
    wxArrayString files;
    wxRegEx re(filespec);
    int files_count = wxDir::GetAllFiles(path, &files, wxEmptyString, wxDIR_FILES);

    if(files_count > 0)
    {
        for(int i = 0; i < files_count; ++i)
        {
            if(re.Matches(files.Item(i)))
            {
                return files.Item(i);
            }
        }
    }

    wxDir dir(path);

    if(dir.IsOpened())
    {
        wxString subdir;

        if(dir.GetFirst(&subdir, wxEmptyString, wxDIR_DIRS))
        {
            do{
                wxFileName fn(path, subdir);
                wxString ret = SearchSpecifiedFile(fn.GetFullPath(), filespec);
                if(ret.IsEmpty() == false)
                {
                    return ret;
                }
            }while(dir.GetNext(&subdir));
        }

    }

    return wxEmptyString;

}

void J5MergeFrame::LoadSrcFilesFromDir(const wxString& path)
{
    wxString fullname = SearchSpecifiedFile(path, "u-boot.bin");
    if(fullname.IsEmpty() == false)
    {
        text_uboot_src_path->SetValue(fullname);
        text_uboot_version->SetValue(GetJ5FileVersion(0x55AA0000, fullname));
    }

    fullname = SearchSpecifiedFile(path, "rd-cramfs.bin");
    if(fullname.IsEmpty() == false)
    {
        text_fs_src_path->SetValue(fullname);
        text_fs_version->SetValue(GetJ5FileVersion(0x55AA0002, fullname));
    }
    
    fullname = SearchSpecifiedFile(path, "uImage");
    if(fullname.IsEmpty() == false)
    {
        text_uimage_src_path->SetValue(fullname);
        wxString uimage_version = GetJ5FileVersion(0x55AA0001, fullname);
        if(uimage_version.Cmp(wxT("FF.FF.FF.FF")) == 0)
        {
            wxString fs_version = text_fs_version->GetValue();
            if(fs_version.Length() > 0)
            {
                text_uimage_version->SetValue(fs_version);
            }
        }
    }

    fullname = SearchSpecifiedFile(path, "SMC.bin");
    if(fullname.IsEmpty() == false)
    {
        text_smc_src_path->SetValue(fullname);
        text_smc_version->SetValue(GetJ5FileVersion(0x55AA0005, fullname));
    }

    fullname = SearchSpecifiedFile(path, "LUT.bin");
    if(fullname.IsEmpty() == false)
    {
        text_lut_src_path->SetValue(fullname);
        text_lut_version->SetValue(GetJ5FileVersion(0x55AA0006, fullname));
    }

    fullname = SearchSpecifiedFile(path, ".*\\.xe674");
    if(fullname.IsEmpty() == false)
    {
        text_dsp_src_path->SetValue(fullname);
        text_dsp_version->SetValue(GetJ5FileVersion(0x55AA0003, fullname));
    }

    fullname = SearchSpecifiedFile(path, ".*\\.xem3");
    if(fullname.IsEmpty() == false)
    {
        text_m3_src_path->SetValue(fullname);
        text_m3_version->SetValue(GetJ5FileVersion(0x55AA0004, fullname));
    }
}

bool J5MergeFrame::LoadAppConfigs()
{
    if(config.ReadFromFile(cfg_path))
    {
        std::string val;
        Setting& root = config.getRoot();
        const char* key = "last_source_dir";
        if(root.exists(key))
        {
            root.lookupValue(key, val);
            recent_source_dir = val.c_str();
        }
        else
        {
            root.add(key, Setting::TypeString);
        }
        key = "last_autoload_dir";
        if(root.exists(key))
        {
            root.lookupValue(key, val);
            recent_autoload_dir = val.c_str();
        }
        else
        {
            root.add(key, Setting::TypeString);
        }
    }

    return false;
}

bool J5MergeFrame::SaveAppConfigs()
{
    Setting& root = config.getRoot();
    const char* key = "last_source_dir";

    Setting* src_setting;
    if(root.exists(key))
    {
        src_setting = &root.lookup(key);
    }
    else
    {
        src_setting = &root.add(key, Setting::TypeString);
    }
    *src_setting = recent_source_dir.mbc_str();

    key = "last_autoload_dir";
    Setting* autoload_setting;
    if(root.exists(key))
    {
        autoload_setting = &root.lookup(key);
    }
    else
    {
        autoload_setting = &root.add(key, Setting::TypeString);
    }
    *autoload_setting = recent_autoload_dir.mbc_str();

    config.SaveToFile(cfg_path);
    return false;
}

void J5MergeFrame::RetsetControls()
{
    wxTextCtrl* src_texts[] = {text_uboot_src_path,text_dsp_src_path,text_m3_src_path,text_uimage_src_path,text_fs_src_path, text_smc_src_path, text_lut_src_path};
    wxTextCtrl* version_texts[] = {text_uboot_version,text_dsp_version,text_m3_version,text_uimage_version,text_fs_version, text_smc_version, text_lut_version};

    for(size_t i = 0; i < sizeof(src_texts)/sizeof(wxTextCtrl*); ++i)
    {
        src_texts[i]->SetValue(wxEmptyString);
    }
    for(size_t i = 0; i < sizeof(version_texts)/sizeof(wxTextCtrl*); ++i)
    {
        version_texts[i]->SetValue(wxEmptyString);
    }

}

void J5MergeFrame::OnMenuShowLog(wxCommandEvent& event)
{
    wxSizerItem* log_sizer = GetSizer()->GetItemById(XRCID("log_sizeritem"), true);
    if(log_sizer != NULL)
    {
        bool show = log_sizer->IsShown();
        log_sizer->Show(!show);
        Layout();
    }
}

void J5MergeFrame::OnMenuClearLog(wxCommandEvent& event)
{
    if(text_runlog != NULL)
    {
        text_runlog->Clear();
    }
}
