unit RegForm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls;

type
  TRegRipper = class;

  WorkerThread = class(TThread)
  private
    _form : TRegRipper;
    _scrollwidth: integer;
    _posttext : string;
    procedure postproc;
    procedure resetbutton;
    procedure post(s:string);
  protected
    procedure Execute; override;
  public
    constructor Create(CreateSuspended: Boolean; form: TRegRipper);
  end;

  TRegRipper = class(TForm)
    SubjURL: TEdit;
    DeptURL: TEdit;
    Source: TGroupBox;
    Label1: TLabel;
    Label2: TLabel;
    GroupBox1: TGroupBox;
    filename: TEdit;
    GroupBox2: TGroupBox;
    Status: TListBox;
    Go: TButton;
    autoscroll: TCheckBox;
    browse: TButton;
    Label3: TLabel;
    SaveDialog: TSaveDialog;
    procedure GoClick(Sender: TObject);
    procedure browseClick(Sender: TObject);
    procedure OnCreate(Sender: TObject);
    procedure OnClose(Sender: TObject; var Action: TCloseAction);
  private
    worker: WorkerThread;
  public
    { Public declarations }
  end;

var
  rr: TRegRipper;

implementation

uses RegHelper;

{$R *.DFM}

procedure TRegRipper.GoClick(Sender: TObject);
begin
 if ((worker=nil) or (worker.Terminated)) then worker := WorkerThread.Create(true,self);

 if (worker.Suspended) then
   begin
     worker.Resume();
     self.Go.Caption := 'Pause';
   end
 else
  begin
    worker.Suspend();
    self.Go.Caption := 'Continue';
  end;
end;

{ WorkerThread }

constructor WorkerThread.Create(CreateSuspended: Boolean; form: TRegRipper);
begin
  _form := form;
  _scrollwidth := 0;
  inherited Create(CreateSuspended);
end;

procedure WorkerThread.postproc;
var i : integer;
begin
  i := _form.Status.Canvas.TextWidth(_posttext);
  if (_scrollwidth < i) then
  begin
    _scrollwidth := i+10;
    SendMessage(_form.Status.Handle, LB_SETHORIZONTALEXTENT, _scrollwidth, 0);
    _form.Status.Invalidate;
  end;
  _form.Status.Items.Append(_posttext);
  if _form.autoscroll.Checked then _form.Status.ItemIndex := _form.Status.Items.Count-1;
end;

procedure WorkerThread.resetbutton;
begin
  _form.Go.Caption := 'Go!'
end;

procedure WorkerThread.post(s:string);
begin
  _posttext := s;
  Synchronize(postproc);
end;

procedure WorkerThread.Execute;
var spider: TSpider;
    s,c,u: string;
    i,j: integer;
    k: char;
    f: Text;
label byebye;
begin
  //i := 0; repeat inc(i); post(format('%d',[i])); WaitForSingleObject(self.Handle,300); if Terminated then goto byebye; until false;
  s := _form.DeptURL.Text;
  spider := TSpider.Create();
  post('----- Constructing Department Map -----');
  for k := 'A' to 'Z' do
  begin
    u := s + 'dept-' + k + '.html';
    post('Downloading ' + u);
    spider.ParseDeptPage(geturl(u));
    if Terminated then goto byebye;
  end;
  post('----- Getting Class Information -----');
  s := _form.SubjURL.Text;
  post('Downloading ' + s);
  spider.ParseSubjectList(geturl(s),s);
  AssignFile(f,_form.filename.Text);
  Rewrite(f);
  for i := 0 to spider.subjectlist.Count - 1 do
  begin
    c := SubjectItem(spider.subjectlist.items[i]).url;
    post('Downloading ' + c);
    spider.ParseClassList(geturl(c),c);
    for j := 0 to spider.classlist.Count-1 do
    begin
      u := ClassItem(spider.classlist.items[j]).url;
      post('Downloading ' + u);
      spider.ParseClassPage(GetURL(u));
      post('  coursename = ' + '''' + spider.classinfo.coursename + '''');
      post('  coursedesc = ' + '''' + spider.classinfo.coursedesc + '''');
      post('  instructor = ' + '''' + spider.classinfo.instructor + '''');
      post('  department = ' + '''' + spider.classinfo.department + '''');
      post('  students = '   + '''' + spider.classinfo.students   + '''');
      post('  subject = '    + '''' + spider.classinfo.subject    + '''');
      post('  division = '   + '''' + spider.classinfo.division   + '''');
      post('  school = '     + '''' + spider.classinfo.school     + '''');
      post('  year = '       + '''' + spider.classinfo.year       + '''');
      post('  semester = '   + '''' + spider.classinfo.semester   + '''');
      post('  subj = '       + '''' + spider.classinfo.subj       + '''');
      post('  courseno = '   + '''' + spider.classinfo.courseno   + '''');
      post('  section = '    + '''' + spider.classinfo.section    + '''');
      post('  dept = '       + '''' + spider.classinfo.dept       + '''');
      write(f,CSVify(spider.classinfo.coursename),',');
      write(f,CSVify(spider.classinfo.coursedesc),',');
      write(f,CSVify(spider.classinfo.instructor),',');
      write(f,CSVify(spider.classinfo.department),',');
      write(f,CSVify(spider.classinfo.students),',');
      write(f,CSVify(spider.classinfo.subject),',');
      write(f,CSVify(spider.classinfo.division),',');
      write(f,CSVify(spider.classinfo.school),',');
      write(f,CSVify(spider.classinfo.year),',');
      write(f,CSVify(spider.classinfo.semester),',');
      write(f,CSVify(spider.classinfo.subj),',');
      write(f,CSVify(spider.classinfo.courseno),',');
      write(f,CSVify(spider.classinfo.section),',');
      writeln(f,CSVify(spider.classinfo.dept));
      if Terminated then goto byebye;
    end;
  end;
  Post('Done.');
  byebye:
  if (not Terminated) then
  begin
    Post(' ');
    Terminate();
  end
  else
    Post('It is over for me.');
  CloseFile(F);
  Synchronize(ResetButton);
  //close handles
end;

procedure TRegRipper.browseClick(Sender: TObject);
begin
  SaveDialog.FileName := filename.Text;
  SaveDialog.Execute();
  filename.Text := SaveDialog.FileName;
end;

procedure TRegRipper.OnCreate(Sender: TObject);
begin
  filename.Text := GetAppPath() + '\registrar.csv';
end;

procedure TRegRipper.OnClose(Sender: TObject; var Action: TCloseAction);
begin
 if (worker <> nil) then
  begin
    worker.Terminate;
    if worker.Suspended then worker.Resume;
    worker.WaitFor;
    worker.Destroy;
  end;
end;

end.







