program regripper;

uses
  Forms,
  RegForm in 'RegForm.pas' {RegRipper},
  reghelper in 'reghelper.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TRegRipper, rr);
  Application.Run;
end.


