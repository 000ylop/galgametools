program ChocoCut;

uses
  Forms,
  kaguya in 'kaguya.pas' {Form1},
  Common in 'Common.pas',
  BMPScreen in 'BMPScreen.pas' {Form2},
  kaguyaCG in 'kaguyaCG.pas',
  Phrase in 'Phrase.pas' {PhraseDlg};

{$R *.RES}

begin
  Application.Initialize;
  Application.Title := 'KaguyaCut';
  Application.CreateForm(TForm1, Form1);
  Application.CreateForm(TForm2, Form2);
  Application.CreateForm(TPhraseDlg, PhraseDlg);
  Application.Run;
end.
