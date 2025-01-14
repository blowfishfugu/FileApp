//---------------------------------------------------------------------------
#if defined BUILD_WITH_VCL || BUILD_WITH_FMX
#include <windows.h>
#include <tchar.h>


#if defined BUILD_WITH_VCL
	#include <vcl.h>
#elif defined BUILD_WITH_FMX
	#include <fmx.h>
#endif

#pragma hdrstop
//---------------------------------------------------------------------------
USEFORM("MainForm.cpp", frmMain);
//---------------------------------------------------------------------------


int WINAPI _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
    try
    {
         Application->Initialize();
         Application->MainFormOnTaskBar = true;
         Application->CreateForm(__classid(TfrmMain), &frmMain);
         Application->Run();
    }
    catch (Exception &exception)
    {
         Application->ShowException(&exception);
    }
    catch (...)
    {
         try
         {
             throw Exception("");
         }
         catch (Exception &exception)
         {
             Application->ShowException(&exception);
         }
    }
    return 0;
}
//---------------------------------------------------------------------------
#else

#endif