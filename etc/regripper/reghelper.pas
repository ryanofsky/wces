unit reghelper;

interface

uses hash,classes;

// ------------------------------------------------------------ Useful Functions

function GetURL(url:string): string;
function GetAppPath():string;
function CSVify(s:string):string;

// ---------------------------------------------------------- Data Storage Types

type TClassinfo = record
    course: string;         // course name
    coursecode: string;     // 4 digit course number
    department: string;     // department name
    departmentcode: string; // 4 letter department abbreviation. Not on page, taken from department hash table deptlist
    subject: string;        // full subject name
    subjectcode: string;    // 4 letter subject abbreviation
    division: string;       // division name
    divisioncode:string;    // 1 or 2 letter division code
    divisionscode:string;   // 1 letter division code
    classname: string;      // class name
    classsection: string;   // 3 digit section ie  '001', '002', 'R01'
    year: string;           // four digit year
    semester: string;       // 'spring', 'fall', 'summer'
    instructor: string;     // full name
    students: string;       // number of students
    school: string;         // school name
    time: string;           // class time
    location:string;        // class location
    callnumber:string;      // 5 digit call number
  end;

type SubjectItem = class(TCollectionItem)
  public url:string;
end;

type ClassItem = class(TCollectionItem)
  public url:string;
end;

// ----------------------------------------------------------- Main Helper Class

type TSpider = class
  public
  deptlist: TStringKeyHashTable; // used to map department names (key) into four letter codes (data)
  subjectlist: TCollection; // stores a list of subjects & urls from most recently parsed directory listing
  classlist: TCollection;  // stores a list of classes & urls from most recently parsed directory listing
  classinfo: TClassinfo;  // stores information about individual classes from most recently parsed class page
  procedure ParseDeptPage(page:string);
  procedure ParseSubjectList(page:string; baseurl:string);
  procedure ParseClassList(page:string; baseurl:string);
  procedure ParseClassPage(page:string);
  destructor Destroy; override;
end;

implementation

uses Windows, wininet, Messages, SysUtils, Graphics, Controls, Forms, Dialogs, StdCtrls;
type
  WinException = class
    private
      _err:longword;
      _msg:string;
    public
      constructor create (msg:string);
      procedure show();
end;

constructor WinException.create (msg:string);
begin
  _err  := GetLastError();
  _msg := msg;
end;

procedure WinException.show();
var b1: string;
var b2: array[0..255] of Char;
begin
  b1 := Format(_msg + ', Error %d',[_err]);
  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,nil,_err,0,@b2,255,nil);
  MessageBox (0,@b2,pchar(b1),MB_ICONEXCLAMATION or MB_OK);
end;

function GetURL(url:string): string;
  const buffersize = 1024;
  var buffer: array[0..buffersize] of Char;
  var return: string;
  var ihandle: HINTERNET;
  var fhandle: HINTERNET;
  var size: DWORD;
begin
  return := '';
  ihandle := InternetOpen('regripper 1.1',INTERNET_OPEN_TYPE_PRECONFIG,nil,nil,0);
  fhandle := InternetOpenUrl(ihandle,pchar(url),nil,0,INTERNET_FLAG_RAW_DATA, 0);
  try
    try
      repeat
        if (not InternetReadFile(fhandle,@buffer,buffersize,size)) then Raise WinException.create('InternetReadFile Failed');
        buffer[size] := Char(0);
        return := return + buffer;
      until size = 0;
    except
      on e:WinException do e.show;
    end;
  finally
    InternetCloseHandle(fhandle);
    InternetCloseHandle(ihandle);
    GetURL := return;
  end;
end;

function GetAppPath():string;
var name:array[0..255] of char; l:integer;
begin
  l := GetModuleFileName(HInstance,@name,255) - 1;
  while(name[l] <> '\') do dec(l);
  name[l] := char(0);
  GetAppPath := string(name);
end;

function CSVify(s:string):string;
var i:integer;
    quoted:boolean;
begin
  quoted := false;
  i := 1;
  while i <= length(s) do
  begin
    if (ord(s[i]) < 32) then s[i] := ' ';
    if (not quoted) and ((s[i] = ',') or (s[i] = '"')) then
    begin
      s := '"' + s + '"';
      inc(i);
      quoted := true;
    end;
    if (s[i] = '"') and not((i=length(s)) and quoted) then
    begin
      insert('"',s,i);
      inc(i);
    end;
    inc(i);
  end;
  result := s;
end;

function StringPos(subst: string; s: string; startat:integer):integer;
{
   StringPos searches for a substring, subst, within a string, s,
   and returns the position where it finds the first occurence.
   It is exactly like pascal's Pos function except that it includes
   a third parameter, startat, which lets the search begin from
   from the middle of the string instead of at the beginning.
}
var p:integer;
begin
  if startat < 1 then startat := 1;
  if (startat > length(s)) then
    StringPos := 0
  else if (subst = '') then
    StringPos := 1
  else
  begin
    p := pos(subst,Pchar(@s[startat]));
    if (p>0) then StringPos := p + startat -1 else StringPos := 0;
  end;
end;

function StripHTML(s:string):string;
{
   StripHTML will remove any text that falls between '<' and '>'
   in a string. It replaces any whitespace in the middle of a string
   with a single space (' ') and trims all whitespace from the
   beginning and end.
}
var i:integer;
    return: string;
    goodstuff,postspace: boolean;
begin
  goodstuff := true;
  postspace := false;
  return := '';
  for i := 1 to length(s) do
  begin
    if s[i]='<' then
      goodstuff := false
    else if s[i]='>' then
      goodstuff := true
    else if goodstuff then
    begin
      if (ord(s[i]) <= 32) then
      begin
        if length(return)>0 then postspace := true;
      end
      else
      begin
        if postspace then begin return := return + ' '; postspace := false; end;
        return := return + s[i];
      end
    end;
  end;
  StripHTML := return;
end;

function KeepNumbers(s:string):string;
{
  Returns the digits at the beginning of a string
}
var i:integer;
    return:string;
begin
  return := '';
  if (length(s) >0) then
  begin
    i := 1;
    while (('0' <= s[i]) and (s[i] <= '9')) do
    begin
      return := return + s[i];
      inc(i);
    end;
  end;
  KeepNumbers := return;
end;

function getvalue(findfirst,opener,closer,page:string; startat:integer):string;
{
  GetValue is a function that can be used to extract a substring from a
  larger string. It is only useful when the substring is surrounded by
  the opener and closer strings and preceded by the findfirst string.
  For example,

     getvalue('heading', '(' , ')' , 'junk heading morejunk  (value)', 0)

  returns 'value' because it is between the parentheses and after the
  heading.

  Delphi does not include any built in support for regular expressions.
}
var spos,epos:integer;
begin
  getvalue := '';
  spos := StringPos(findfirst,page,startat);
  if (spos<1) then exit;
  spos := StringPos(opener,page,spos);
  if (spos<1) then exit;
  epos := StringPos(closer,page,spos);
  if (epos<spos) then exit;
  getvalue := Copy(page,spos+length(opener),epos-spos-length(opener));
end;

procedure TSpider.ParseDeptPage(page:string);
var no,nc,vo,vc: string;
    deptname, deptabbv:string;
    pos1,pos2,pos3,pos4,pos5:integer;
begin
  if deptlist = nil then deptlist := TStringKeyHashTable.Create;

  no := '<tr valign=top><td bgcolor=#DADADA>';
  nc := '</td>';
  vo := '<a href="';
  vc := '">';
  pos2 := 1;
  repeat
    pos1 := StringPos(no,page,pos2);
    pos2 := StringPos(nc,page,pos1);
    deptname := Copy(page,pos1 + length(no), pos2 - pos1 - length(no));
    pos3 := Stringpos(vo,page,pos2);
    pos4 := Stringpos(vc,page,pos3);
    pos5 := Stringpos('_',page,pos3);
    if (pos3 + length(vo) + 4 <= pos5) and (pos5 < pos4) then
      deptabbv := Copy(page,pos5-4,4)
    else
      deptabbv := '';
    deptlist.Value[deptname] := deptabbv;
  until (pos1 = 0) or (pos2=0)
end;

procedure TSpider.ParseSubjectList(page:string; baseurl:string);
var item: SubjectItem;
    opener,closer: string;
    pos1,pos2: integer;
begin
  if subjectlist <> nil then subjectlist.Destroy;
  subjectlist := TCollection.Create(SubjectItem);
  opener := '<IMG SRC="/icons/folder.gif" ALT="[DIR]"> <A HREF="';
  closer := '/">';
  pos2 := 1;
  repeat
    pos1 := StringPos(opener,page,pos2);
    pos2 := StringPos(closer,page,pos1);
    if pos2 - pos1 - length(opener) = 4 then
    begin
     item := SubjectItem(subjectlist.add());
     item.url := baseurl + Copy(page,pos1+length(opener),pos2-pos1-length(opener)) + '/';
    end;
  until (pos1 = 0) or (pos2 = 0);
end;

procedure TSpider.ParseClassList(page:string; baseurl:string);
var item: ClassItem;
    opener,closer: string;
    pos1,pos2: integer;
begin
  if classlist <> nil then classlist.Destroy;
  classlist := TCollection.Create(ClassItem);
  opener := '<IMG SRC="/icons/folder.gif" ALT="[DIR]"> <A HREF="';
  closer := '/">';
  pos2 := 1;
  repeat
    pos1 := StringPos(opener,page,pos2);
    pos2 := StringPos(closer,page,pos1);
    if pos2 - pos1 - length(opener)= 15 then
    begin
     item := ClassItem(classlist.add());
     item.url := baseurl + Copy(page,pos1+length(opener),pos2-pos1-length(opener)) + '/';
    end;
  until (pos1 = 0) or (pos2 = 0);
end;

procedure TSpider.ParseClassPage(page:string);
var titleopen,tableopen,pos:integer;
    no,nc,vo,vc:string;
    key,coords:string;
begin
  no := '<tr valign=top><td bgcolor=#99CCFF>'; // name opener
  nc := '</td>';                               // name closer
  vo := '<td bgcolor=#DADADA>';                // value opener
  vc := '</td></tr>';                          // value closer
  titleopen  := StringPos('<td colspan=2 bgcolor=#99CCFF><b><br>',page,1);
  tableopen  := StringPos('<tr valign=top><td bgcolor=#99CCFF>',page,1);
  classinfo.course := getvalue('','<font size=+2>','</font><br>',page,titleopen);
  classinfo.classname := StripHTML(getvalue('<font size=+2>','</font><br>','<tr valign=top><td bgcolor=#99CCFF>',page,titleopen));
  classinfo.instructor := getvalue(no+'Instructor'+nc,vo,vc,page,tableopen);
  classinfo.department := StripHTML(getvalue(no+'Department'+nc,vo,vc,page,tableopen));
  classinfo.students   := KeepNumbers(getvalue(no+'Enrollment'+nc,vo,vc,page,tableopen));
  classinfo.subject    := getvalue(no+'Subject' + nc,vo,vc,page,tableopen);
  classinfo.division   := getvalue(no+'Division' + nc,vo,vc,page,tableopen);
  classinfo.school     := StripHTML(getvalue(no+'School'+nc,vo,vc,page,tableopen));
  classinfo.callnumber := getvalue(no + 'Call Number' + nc,vo,vc,page,tableopen);
  classinfo.divisioncode := getvalue(no + 'Number' + nc,vo,vc,page,tableopen);
  classinfo.divisioncode := Copy(classinfo.divisioncode,1,length(classinfo.divisioncode)-4);
  key := getvalue(no+'Section key'+nc,vo,vc,page,tableopen);
  if length(key) = 17 then
  begin
    classinfo.year := Copy(key,1,4);
    case key[5] of
      '3': classinfo.semester := 'fall';
      '2': classinfo.semester := 'summer';
      '1': classinfo.semester := 'spring';
    else
      classinfo.semester := 'unknown';
    end;
    classinfo.subjectcode := Copy(key,6,4);
    classinfo.coursecode := Copy(key,10,4);
    classinfo.divisionscode := string(key[14]);
    classinfo.classsection := Copy(key,15,3);
  end;
  classinfo.departmentcode := string(deptlist.Value[classinfo.department]);
  coords := getvalue(no + 'Day &amp; Time<br>Location' + nc,vo,vc,page,tableopen);
  pos := StringPos('<br>',coords,1);
  classinfo.time := Copy(coords,0,pos-1);
  classinfo.location := Copy(coords,pos+4,length(coords)-pos-3);
end;

destructor TSpider.Destroy;
begin
  if (deptlist <> nil) then deptlist.Destroy;
  if (subjectlist <> nil) then subjectlist.Destroy;
  if (classlist <> nil) then classlist.Destroy;
end;

end.
