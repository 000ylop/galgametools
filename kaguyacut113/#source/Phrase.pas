unit Phrase;

interface

uses
  Windows, SysUtils, Classes, Graphics, Forms, Controls, StdCtrls,
  Buttons, ExtCtrls, kaguya;

type
  TPhraseDlg = class(TForm)
    OKBtn: TButton;
    CancelBtn: TButton;
    PhraseEdit: TEdit;
    Label1: TLabel;
    procedure CancelBtnClick(Sender: TObject);
    procedure OKBtnClick(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  PhraseDlg: TPhraseDlg;

implementation

{$R *.dfm}

procedure TPhraseDlg.CancelBtnClick(Sender: TObject);
begin
  PhraseDlg.Close;
end;

procedure TPhraseDlg.OKBtnClick(Sender: TObject);
begin
  if PhraseEdit.Text<>'' then
  begin
    PhraseText:=PhraseEdit.Text;
    Form1.ComparePhrase;
  end;
end;

end.
