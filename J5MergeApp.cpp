/***************************************************************
 * Name:      J5MergeApp.cpp
 * Purpose:   Code for Application Class
 * Author:    知遇 (178041876@qq.com)
 * Created:   2017-02-14
 * Copyright: 知遇 ()
 * License:
 **************************************************************/

#ifdef WX_PRECOMP
#include "wx_pch.h"
#endif

#ifdef __BORLANDC__
#pragma hdrstop
#endif //__BORLANDC__

#include "J5MergeApp.h"
#include "J5MergeMain.h"

#include <wx/xrc/xmlres.h>

#include <wx/process.h>
#include <wx/stream.h>

IMPLEMENT_APP(J5MergeApp);

class MySimpleProcess : public wxProcess
{
public:
    virtual void OnTerminate(int pid, int status)
    {
    }
};
static void Process(char* buffer, int size)
{
    for(int i = 0; i < size-1; ++i)
    {
        if(buffer[i] == 0)
            buffer[i] = ' ';
    }
}

typedef void (*Func)();
class A
{
public:
A()
{
    Func p = (Func)&A::test;
}
void test()
{
}
};

bool J5MergeApp::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;
    wxXmlResource::Get()->InitAllHandlers();

    #ifndef _WIN32
    char path[256] = {0};
    readlink("/proc/self/exe", path, 256);
    wxFileName name(path);
    wxSetWorkingDirectory(name.GetPath());
    #endif

    if ( !wxXmlResource::Get()->LoadAllFiles("rc") )
        return false;

    J5MergeFrame* frame = new J5MergeFrame(0L, _("wxWidgets Application Template"));

    frame->Show();

    return true;
}
