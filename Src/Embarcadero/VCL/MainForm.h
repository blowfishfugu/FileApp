//---------------------------------------------------------------------------

#ifndef MainFormH
#define MainFormH

#if defined BUILD_WITH_VCL

//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.ExtCtrls.hpp>
#endif

//---------------------------------------------------------------------------
#include "Process.h"

#if defined BUILD_WITH_VCL

class TfrmMain : public TForm
{
__published:	// Von der IDE verwaltete Komponenten
    TPanel *Panel1;
    TPanel *Panel2;
    TPanel *Panel3;
    TLabel *lblDirectory;
    TEdit *edtDirectory;
    TButton *btnCount;
    TButton *btnParse;
    TStatusBar *sbMain;
    TMemo *memError;
    TListView *lvOutput;
    TSplitter *Splitter1;
    TButton *btnShow;
    TListBox *lbValues;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall btnCountClick(TObject *Sender);
    void __fastcall btnParseClick(TObject *Sender);
    void __fastcall btnShowClick(TObject *Sender);
private:	// Benutzer-Deklarationen
    TProcess proc;
public:		// Benutzer-Deklarationen
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
#endif