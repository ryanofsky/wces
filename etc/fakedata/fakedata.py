from pyPgSQL import PgSQL
import random
import re
import os

DATABASE = "wces"
SUBJECTS = (("GEO", "Geology", ("Nesis", "Plesis")),
            ("SEI", "Seismology", ("Dentrate", "Conflate")),
            ("ELE", "Electrology", ("Plestro", "Festo")))
COURSES = ("Introduction to %s",
           "Advanced %s",
           "Seminar In Applied %s")
ADMIN_FNAME = "Andrew T."
ADMIN_LNAME = "Administrator"
PROF_FNAMES = ("Vestuvius", "Manjeta", "Laureena", "Nervante", "Cooletto")
PROF_LNAMES = ("Bolingrove", "Cockingham", "Peresanto", "Ninsemburg", "Faresh")
STUD_FNAMES = ("Neenosh", "Farlo", "Tendrum", "Padril", "Cerfake", "Ichla")
STUD_LNAMES = ("Beep", "Tam", "Dool", "Fair", "Gorb", "Yelk", "Fide", "Slam")
EMAIL_SUFFIX = "@columbia.edu"
NUM_STUDENTS = 100
NUM_PROFESSORS = 20
NUM_CLASSES = 20
STUDENTS_PER_CLASS = (20, 10)

if 0 and True:
  NUM_STUDENTS=5
  NUM_PROFESSORS=2
  NUM_CLASSES=2
  STUDENTS_PER_CLASS = (3, 0)

YEAR = 2003
SEMESTER = 2

class kw:
  def __init__(self, **args):
    vars(self).update(args)

def question(text):
  return kw(table="components_choice_question", 
            cols={"type": 6,
                  "qtext": text},
            children=None)
            
BASE_SURVEY = kw(table="components_survey", cols={"type": 1},
                 children=[kw(table="components_choice",
                              cols={"type": 2,
                                    "ctext": "",
                                    "flags": 0,
                                    "choices": PgSQL.PgArray
                                      (["excellent", "very good",
                                        "satisfactory", "poor", "disastrous"]),
                                    "other_choice": None,
                                    "first_number": 5,
                                    "last_number": 1,
                                    "rows": 0},
                              children=map(question,
                                ("Instructor: Organization and Preparation",
                                 "Instructor: Classroom Delivery",
                                 "Instructor: Approachability",
                                 "Instructor: Overall Quality",
                                 "Course: Amount Learned",
                                 "Course: Appropriateness of Workload",
                                 "Course: Fairness of Grading Process",
                                 "Course: Overall Quality"))),
                           kw(table="components_text_question",
                              cols={"type": 3,
                                    "ctext": "Comments:",
                                    "flags": 0,
                                    "rows": 5, 
                                    "cols": 60},
                             children=None)])

QUESTION_PERIODS = [kw(cols={"displayname": "Fall 2003 Midterm Evaluations",
                             "begindate": PgSQL.Timestamp(2003, 10, 15),
                             "enddate":  PgSQL.Timestamp(2003, 11, 1),
                             "year": YEAR,
                             "semester": SEMESTER,
                             "profdate": PgSQL.Timestamp(2003, 11, 1),
                             "oracledate": PgSQL.Timestamp(2003, 11, 1)},
                       response_rate=(.9, .1)),
                    kw(cols={"displayname": "Fall 2003 Final Evaluations",
                             "begindate": PgSQL.Timestamp(2003, 12, 1),
                             "enddate": PgSQL.Timestamp(2010, 12, 15),
                             "year": YEAR,
                             "semester": SEMESTER,
                             "profdate": PgSQL.Timestamp(2010, 12, 15),
                             "oracledate": PgSQL.Timestamp(2010, 12, 15)},
                       response_rate=(.3, .1))]

def fakedata():
  conn = db_connect()
  cursor = conn.cursor()
 
  # -- Administrator -- #
  admin_id = db_insert(cursor, "users", "user_ids",
                       uni="admin",
                       lastname=ADMIN_LNAME,
                       firstname=ADMIN_FNAME,
                       email="admin%s" % EMAIL_SUFFIX,
                       lastlogin=None,
                       flags=0x1)
 
  # -- Professors -- #
  professors = []
  for i in range(1, 1+NUM_PROFESSORS):
    user_id = db_insert(cursor, "users", "user_ids",
                        uni="prof%i" % i,
                        lastname=random.choice(PROF_LNAMES),
                        firstname=random.choice(PROF_FNAMES),
                        email="prof%i%s" % (i, EMAIL_SUFFIX),
                        lastlogin=None,
                        flags=0x4)
    professors.append(user_id)

  # -- Students -- #
  students = []
  for i in range(1, 1+NUM_STUDENTS):
    user_id = db_insert(cursor, "users", "user_ids",
                        uni="student%i" % i,
                        lastname=random.choice(STUD_LNAMES),
                        firstname=random.choice(STUD_FNAMES),
                        email="student%i%s" % (i, EMAIL_SUFFIX),
                        lastlogin=None,
                        flags=0x8)
    students.append(user_id)

  # -- Courses -- #
  courses = {}
  for code, name, subsubjects in SUBJECTS:
    subject_id = db_insert(cursor, "subjects", "subject_ids",
                           code=code, name=name)

    course_no = 0
    for name in COURSES:
      course_no += 1000
      subsub_no = 0
      for subsubject in subsubjects:
        subsub_no += 10
        course_id = db_insert(cursor, "courses", "course_ids",
                              subject_id=subject_id,
                              code=course_no + subsub_no,
                              name=name % subsubject,
                              divisioncode='')
        courses[course_id] = 0

  # -- Classes & Enrollments-- #
  classes = []
  for i in range(NUM_CLASSES):
    course_id = random.choice(courses.keys())
    section = courses[course_id] = courses[course_id] + 1
    num_students = int(random.normalvariate(*STUDENTS_PER_CLASS))
    if num_students < 0:
      num_students = 0
    class_id = db_insert(cursor, "classes", "class_ids",
                         course_id=course_id,
                         section="%03i" % section,
                         year=YEAR,
                         semester=SEMESTER,
                         students=num_students)

    db_call(cursor, "enrollment_update", random.choice(professors), 
            class_id, 4, None)

    class_students = random.sample(students, num_students)
    for student in class_students:
      db_call(cursor, "enrollment_update", student, class_id, 1, None)

    classes.append((class_id, class_students))

  # -- Base Survey -- #
  save_id = db_insert(cursor, "saves", "save_ids", user_id=admin_id)
  base_specialization_id = db_insert(cursor, "specializations",
                                     "specialization_ids", parent=None)
  save_component(cursor, BASE_SURVEY, base_specialization_id, save_id)

  base_topic_id = db_insert(cursor, "wces_base_topics", "topic_ids",
                            name="Base Questions", ordinal=1)
  db_insert(cursor, "wces_topics", None, item_id=BASE_SURVEY.item_id,
            specialization_id=base_specialization_id,
            topic_id=base_topic_id,
            make_public=True, cancelled=False)

  # -- Question Periods & Topics -- #
  topics = {}
  for period in QUESTION_PERIODS:
    period_id = db_insert(cursor, "question_periods", "question_period_ids", 
                          **period.cols)

    for class_id, class_students in classes:
      specialization_id = db_insert(cursor, "specializations",
                                    "specialization_ids",
                                    parent=base_specialization_id)
      topic_id = db_insert(cursor, "wces_topics", "topic_ids",
                          class_id=class_id,
                          question_period_id=period_id,
                          item_id=BASE_SURVEY.item_id,
                          specialization_id=specialization_id,
                          category_id=None,
                          make_public=True,
                          cancelled=False)
      topics[class_id, period_id] = topic_id, specialization_id

    period.period_id = period_id

  # -- Responses -- #
  for period in QUESTION_PERIODS:
    for class_id, class_students in classes:
      topic_id, specialization_id = topics[class_id, period.period_id]
      responses = int(random.normalvariate(*period.response_rate)
                      * len(class_students))
      responses = min(max(responses, 0), len(class_students))
      for user_id in random.sample(class_students, responses):
        respond_component(cursor, BASE_SURVEY, (topic_id, specialization_id,
                          user_id, None))

  db_call(cursor, "cached_choice_responses_update")  
  
  # -- Finish up -- #
  conn.commit()

def save_component(cursor, component, specialization_id, save_id):
  cid = db_insert(cursor, component.table, "component_ids", **component.cols)
  bid = db_insert(cursor, "branches", "branch_ids", parent=None)
  rid = db_insert(cursor, "revisions", "revision_ids",
                  branch_id=bid,
                  revision=1,
                  save_id=save_id,
                  component_id=cid)
  db_update(cursor, "branches", "branch_id", bid, latest_id=rid)
  iid = db_nextval(cursor, "item_ids")
  db_insert(cursor, "item_specializations", None,
            item_id=iid,
            specialization_id=specialization_id,
            branch_id=bid)

  component.item_id = iid
  component.revision_id = rid

  if component.children is not None:
    ordinal = 0
    for _component in component.children:
      ordinal += 1
      save_component(cursor, _component, specialization_id, save_id)
      db_insert(cursor, "component_items", None,
                component_id=cid,
                item_id=_component.item_id,
                ordinal=ordinal)

def respond_component(cursor, component, parent):
  component_type = component.cols["type"]
  children = ()

  if component_type == 1:
    assert type(parent) is tuple
    topic_id, specialization_id, user_id, date = parent
    db_insert(cursor, "responses_survey", None, response_id=None,
              revision_id=component.revision_id, item_id=component.item_id,
              topic_id=topic_id, specialization_id=specialization_id,
              user_id=user_id)
    response_id = db_insert(cursor, "responses_survey", "response_ids",
                            revision_id=component.revision_id,
                            item_id=component.item_id,
                            topic_id=topic_id,
                            specialization_id=specialization_id,
                            date=date)
  
    children = component.children
  
  elif component_type == 2:
    response_id = db_insert(cursor, "responses_choice", "response_ids",
                            revision_id=component.revision_id,
                            item_id=component.item_id,
                            parent=parent)
    for question in component.children:
      answer = random.randint(1, len(component.cols["choices"])) - 1
      db_insert(cursor, "responses_choice_question", None,
                revision_id=question.revision_id,
                item_id=question.item_id,
                parent=response_id, answer=answer, other=None)

  elif component_type == 3:
    answer = random_comment()
    db_insert(cursor, "responses_text_question", None,
              revision_id=component.revision_id,
              item_id=component.item_id,
              parent=parent,
              rtext=answer) 
  else:
    raise Error("Unknown component type %i" % component_type)
  
  for _component in children:
    respond_component(cursor, _component, response_id)


def random_comment():
  fp = os.popen("fortune", "r")
  try:
    fortune = fp.read()
  finally:
    fp.close()
  # get rid of the attribution so they look more look like real comments
  return _re_attrib.sub("", fortune)

_re_attrib = re.compile(r"\s+\-\-.{,100}$", re.DOTALL)

def db_connect():
 return PgSQL.connect("::%s:postgres:::" % DATABASE)

def db_update(cursor, table, id_col, id_val, **fields):
  sets = []
  vals = []

  for field, val in fields.items():
    sets.append("%s = %%s" % field)
    vals.append(val)
  vals.append(id_val)
  
  db_execute(cursor, "UPDATE %s SET %s WHERE %s = %%s" 
                     % (table, ", ".join(sets), id_col), *vals)

def db_insert(cursor, table, id_seq=None, **fields):
  names = []
  vals = []

  for name, val in fields.items():
    names.append(name)
    vals.append(val)

  db_execute(cursor, "INSERT INTO %s (%s) VALUES (%s)"
                     % (table,
                        ", ".join(names),
                        ", ".join(("%s",) * len(vals))),
             *vals)

  if id_seq:
    cursor.callproc("currval", id_seq)
    return cursor.fetchone()[0]

def db_call(cursor, func, *args):
  print "SELECT %s(%s)" % (func, ", ".join(map(PgSQL._quote, args)))
  cursor.callproc(func, *args)
  return cursor.fetchone()[0]

def db_nextval(cursor, seq):
  cursor.callproc("nextval", seq)
  return cursor.fetchone()[0]

def db_execute(cursor, qstr, *parms):
  print qstr % tuple(map(PgSQL._quote, parms))
  cursor.execute(qstr, *parms)

if __name__ == '__main__':
  fakedata()
