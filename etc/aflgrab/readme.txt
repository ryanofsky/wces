AffilGrabber Readme

Contents
----------------------------------------------------

  bin/          compiled version of aflgrab and it's
                associated dlls

  src/          aflgrab source code and project files

  examples/     examples of aflgrab used in
                Coldfusion, ASP, and PHP pages

Installation Instructions (component)
----------------------------------------------------

  Copy the contents of the bin/ folder to a location
  on the web server.

  (If the files are on an NTFS partition and you are
  using IIS and a scripting language other than
  coldfusion, then the IUSR_SERVER username
  must be given read and execute access to the
  files.)

  At a command prompt, change to the directory
  containing the files and type 

    regsvr32 aflgrab.dll

Uninstallation instructions (component)
----------------------------------------------------

  At a command prompt, change to the directory
  containing the files and type 

    regsvr32 /u aflgrab.dll

  The files can then be deleted.

Installation Instructions (examples)
----------------------------------------------------
 
  To run the examples, just copy them into a folder
  on your web server and view the pages in a web
  browser.

  A working copy of the examples is also posted at

    https://yahweh.detour.net/aflgrab/

  The example asks for a AcIS username and password.
  After validation, these are immediately discarded.

About the component
----------------------------------------------------

  Basically all the component does is download a
  web page from a secured web server.

  It has one method and one property.

  To use it, you must first instantiate it. In 
  coldfusion this is done with

  <cfobject TYPE="COM" ACTION="CREATE" NAME="grabber" CLASS="Oracle.AffilGrabber">

  Then you call the 'Validate' method, which takes
  these arguments:

  username - a string containing a username
  password - string containing password
  url - page to download

  The function returns TRUE if the page was
  downloaded successfully and FALSE if the page
  could not be retrieved.

  The results of the HTTP request (including
  headers and page contents) are stored in the
  property called 'rawoutput'

  To use the component to validate a CUNIX
  username and password, you tell the 
  component to download a page protected
  by an AcIS HTTP authentication system.

  I have set up a page like this at

  https://www1.columbia.edu/~rey4/info.html

  If downloaded with a correct username and
  password, the page contains information
  about the username (including class
  enrollments). If an attempt is made to 
  download the page with an invalid username
  and password, the Validate Function returns
  FALSE, and the page is not downloaded
  at all.

  Instructions for creating your own
  protected pages are in the 'src.txt'
  file.

  Examples of how to use the component
  in a web page are in the examples/
  folder.

  Information about the data returned in
  the info.html page is available from 
  AcIS at

  http://www.columbia.edu/acis/rad/authmethods/auth-affils/
  http://www.columbia.edu/acis/rad/authmethods/auth-courses/
  http://www.columbia.edu/acis/rad/authmethods/cheesewhiz/
  http://www.columbia.edu/acis/rad/authmethods/
