RegRipper 1.1 Readme

Contents
----------------------------------------------------

  bin/       RegRipper executable

  src/       Source code and project files

Instructions
----------------------------------------------------

  Run regripper.exe, choose a path to save the
  output, and click "Go!"

  The output will scroll by in the status window
  and be saved into a text file. To turn off 
  automatic scrolling, uncheck the "Enable Auto-
  Scroll" option at the bottom of the window.

Output
----------------------------------------------------

  The text file that is output is in CSV format. 
  This means it can be opened as a spreadsheet 
  in Excel and imported directly into a database.

  For each class, it contains these 19 fields:

  1)  Course Name (text)
  2)  Course Code (4 digits)
  3)  Department Name (text)
  4)  Department Code (4 letters)
  5)  Subject Name (text)
  6)  Subject Code (4 letters)
  7)  Division Name (text)
  8)  Division Code (1-2 Letters)
  9)  Division Prefix (1 Letter)
  10) Class Name (text)
  11) Class Section (3 Letters. ie '001', '002', 'R01')
  12) Year (4 digits)
  13) Semester (either 'spring', 'fall', or 'summer')
  14) Instructor (text, full name. ie 'Rastislav P Levicky')
  15) Enrollment (integer)
  16) School (text)
  17) Time (text. ie 'TR 1:10pm-2:25pm')
  18) Location (text. ie '501 Schermerhorn Hall')
  19) Call Number (5 digits)

  If you are want to access the information from 
  a relational database, you probably will want to
  normalize it by breaking it down into several
  smaller linked tables. For the SEAS Oracle we
  import the data into these tables:

  Classes
  Courses
  Subjects
  Departments
  Schools
  Divisions
  Professors

Links
----------------------------------------------------

  There is some information about how Columbia
  organizes its courses at

    http://www.college.columbia.edu/students/academics/bulletin/depts/index.html

  There is also some less intelligible stuff
  accessible through CUNIX in 

    /wwws/data/cu/bulletin/uwb

  It is partially available through the web at

    http://www.columbia.edu/cu/bulletin/uwb/about/about-update.html

  My email address is rey4@columbia.edu.