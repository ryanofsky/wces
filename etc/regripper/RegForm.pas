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
    SubjectsLabel: TLabel;
    Label2: TLabel;
    DestBox: TGroupBox;
    filename: TEdit;
    StatusBox: TGroupBox;
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
    procedure OnResize(Sender: TObject);
    procedure WMGetMinMaxInfo( var Message :TWMGetMinMaxInfo ); message WM_GETMINMAXINFO;
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
      post('  course = ' + '''' + spider.classinfo.course + '''');
      post('  coursecode = ' + '''' + spider.classinfo.coursecode + '''');
      post('  department = ' + '''' + spider.classinfo.department + '''');
      post('  departmentcode = ' + '''' + spider.classinfo.departmentcode + '''');
      post('  subject = ' + '''' + spider.classinfo.subject + '''');
      post('  subjectcode = ' + '''' + spider.classinfo.subjectcode + '''');
      post('  division = ' + '''' + spider.classinfo.division + '''');
      post('  divisioncode = ' + '''' + spider.classinfo.divisioncode + '''');
      post('  divisionscode = ' + '''' + spider.classinfo.divisionscode + '''');
      post('  classname = ' + '''' + spider.classinfo.classname + '''');
      post('  classsection = ' + '''' + spider.classinfo.classsection + '''');
      post('  year = ' + '''' + spider.classinfo.year + '''');
      post('  semester = ' + '''' + spider.classinfo.semester + '''');
      post('  instructor = ' + '''' + spider.classinfo.instructor + '''');
      post('  students = ' + '''' + spider.classinfo.students + '''');
      post('  school = ' + '''' + spider.classinfo.school + '''');
      post('  time = ' + '''' + spider.classinfo.time + '''');
      post('  location = ' + '''' + spider.classinfo.location + '''');
      post('  callnumber = ' + '''' + spider.classinfo.callnumber + '''');
      write(f,CSVify(spider.classinfo.course),',');
      write(f,CSVify(spider.classinfo.coursecode),',');
      write(f,CSVify(spider.classinfo.department),',');
      write(f,CSVify(spider.classinfo.departmentcode),',');
      write(f,CSVify(spider.classinfo.subject),',');
      write(f,CSVify(spider.classinfo.subjectcode),',');
      write(f,CSVify(spider.classinfo.division),',');
      write(f,CSVify(spider.classinfo.divisioncode),',');
      write(f,CSVify(spider.classinfo.divisionscode),',');      
      write(f,CSVify(spider.classinfo.classname),',');
      write(f,CSVify(spider.classinfo.classsection),',');
      write(f,CSVify(spider.classinfo.year),',');
      write(f,CSVify(spider.classinfo.semester),',');
      write(f,CSVify(spider.classinfo.instructor),',');
      write(f,CSVify(spider.classinfo.students),',');
      write(f,CSVify(spider.classinfo.school),',');
      write(f,CSVify(spider.classinfo.time),',');
      write(f,CSVify(spider.classinfo.location),',');
      writeln(f,CSVify(spider.classinfo.callnumber));
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
  CloseFile(f);
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

procedure TRegRipper.OnResize(Sender: TObject);
var width,height:integer;
    r: TRect;
begin
  r := GetClientRect();
  width := r.right - r.left;
  height := r.bottom - r.top;
  self.Source.Width := width - 16;
  self.SubjURL.Width := width - 132;
  self.DeptURL.Width := width - 132;
  self.DestBox.Width := width - 16;
  self.filename.Width := width - 212;
  self.browse.Left := width - 99;
  self.Go.Left := round((width - self.Go.Width) / 2);
  self.StatusBox.Width := width - 16;
  self.Status.Width := width - 32;
  self.StatusBox.Height := height - 244;
  self.Status.Height :=  height - 292;
  self.autoscroll.Top :=  height - 269;
end;

procedure TRegRipper.WMGetMinMaxInfo( var Message :TWMGetMinMaxInfo );
begin
  with Message.MinMaxInfo^ do
  begin
    ptMinTrackSize.X := 310;
    ptMinTrackSize.Y := 380;
  end;
  Message.Result := 0;
  inherited;  
end;




end.







