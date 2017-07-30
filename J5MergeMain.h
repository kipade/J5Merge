/***************************************************************
 * Name:      J5MergeMain.h
 * Purpose:   Defines Application Frame
 * Author:    知遇 (178041876@qq.com)
 * Created:   2017-02-14
 * Copyright: 知遇 ()
 * License:
 **************************************************************/

#ifndef J5MERGEMAIN_H
#define J5MERGEMAIN_H

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <stdint.h>
#include "common.h"
#include <libconfig.hh>
using namespace libconfig;

#include "J5MergeApp.h"
#include <vector>
using namespace std;



class J5MergeFrame: public wxFrame
{
    typedef struct
    {
        unsigned int id;
        unsigned int offset;//起始偏移
        unsigned int length;//长度
        unsigned int end;//结束位置的下一地址
        const char* name;
    }MergeConfigItem;

    static bool CmpMergeCfgItem(const MergeConfigItem& it1, const MergeConfigItem& it2);

    public:
        J5MergeFrame(wxFrame *frame, const wxString& title);
        ~J5MergeFrame();
    private:
        enum
        {
            idMenuQuit = 1000,
            idMenuLoadDir,
            idMenuAbout
        };

        wxTextCtrl* text_uboot_src_path;
        wxTextCtrl* text_dsp_src_path;
        wxTextCtrl* text_m3_src_path;
        wxTextCtrl* text_uimage_src_path;
        wxTextCtrl* text_fs_src_path;
        wxTextCtrl* text_smc_src_path;
        wxTextCtrl* text_lut_src_path;

        wxTextCtrl* text_uboot_version;
        wxTextCtrl* text_dsp_version;
        wxTextCtrl* text_m3_version;
        wxTextCtrl* text_uimage_version;
        wxTextCtrl* text_fs_version;
        wxTextCtrl* text_smc_version;
        wxTextCtrl* text_lut_version;

        wxString recent_source_dir;
        wxString recent_autoload_dir;
        const char* cfg_path;
        Config config;

		vector<SectionItem> section_items;
		vector<MergeConfigItem> merge_configs;

		SectionItem* GetSection(uint32_t id);
        bool InitControls();
        bool SelectMergeConfig();//选择对应的合成配置(非软件配置)
        bool LoadMergeConfig(const wxString& path); //从文件加载合成选项
        int ParseMergeConfig(const Config& merge_cfg);//从配置对象加载合成选项
        void SortMergeConfigByOffset();//对各配置项按起始地址排序
		bool DoMerge(const wxString& dst);
		bool GenerateCheckedFiles(std::vector<SectionItem>* sections = NULL);
		bool ParseConfig();
		bool GenerateCheckedFile(const wxString& src, const wxString& dst, uint32_t id, const wxString& version);
		wxString PreprocessDspM3(const wxString& src);
		void DeleteAllSectionFiles();
		void LoadSrcFilesFromDir(const wxString& dir);
		bool SaveAppConfigs();
		bool LoadAppConfigs();
		void RetsetControls();

        void OnClose(wxCloseEvent& event);
        void OnQuit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);

        void OnAnyButtonPressed(wxCommandEvent& event);
        void OnGenerateComponent(wxCommandEvent& event);
        void OnMenuLoadFromDir(wxCommandEvent& event);

        DECLARE_EVENT_TABLE()
};


#endif // J5MERGEMAIN_H
