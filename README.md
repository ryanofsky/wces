Web Based Course Evaluation System (WCES)
=========================================

CVS repository: [https://russ.yanofsky.org/viewvc.py/wces](https://russ.yanofsky.org/viewvc.py/wces)

WCES is a project of the Columbia University School of Engineering. It's a web
site that lets administrators and professors create customized online surveys
about courses and see reports showing survey results. The site originally
started as a project for a software engineering class by a group of students I
didn't know. But it was picked up and used by the engineering school, which
hired me in Fall 2000 to work on it part-time. Over time, I added many new
features and reimplemented most of the preexisting functionality to make the
system more flexible. At this point almost all of the code is my own, though I
can't take credit for most of the graphics and text on the site, and I also had
a lot of help dealing with unix administration / server maintenance issues that
came up during development.

The site is mostly implemented in PHP, but there's also a big chunk of core
logic written in procedural SQL. And there are a number of smaller components
written in other languages, including 2 C++ Postgres extensions, a mini web-
crawler written in Delphi, and a COM authentication component written in Visual
C++ with ATL.

Since this is one of the biggest projects I've worked on, I've put up a
demonstration copy of the site at
[https://wces.russ.yanofsky.org/](https://wces.russ.yanofsky.org/).
