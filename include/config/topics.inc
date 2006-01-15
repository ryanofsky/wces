<?

// get constants
require_once('wces/TopicEditor.inc');

function topics_list($question_period_id)
{
  
  /*
  

insert into components_survey (type, ctext, flags) values (1, '', 0);
select currval('component_ids');
 currval 
---------
    2182

insert into saves (user_id) values (null);
INSERT 31609416 1
wces=# select currval('save_ids');
 currval 
---------
     551
     
wces=# select nextval('revision_ids');
 nextval 
---------
    2182
(1 row)

wces=# select nextval('branch_ids');
 nextval 
---------
    1754

insert into branches (branch_id) values (1754);
     
insert into revisions (revision_id, parent, branch_id, revision, save_id, component_id)
values (2182, NULL, 1754, 1, 551, 2182);

update branches set latest_id = 2182 where branch_id = 1754;

  select nextval('item_ids')
wces-# ;
 nextval 
---------
    1627
(1 row)


wces=# select nextval('specialization_ids');
 nextval 
---------
   25134
(1 row)

insert into item_specializations(item_id, specialization_id, branch_id)
values (1627, 25134, 1754);
*/
  
  
  return array
  (  // heading, sql, name type
    array
    ( '<h5>Base Surveys</h5>', 
      "
        SELECT * FROM
        (
          SELECT 0 AS topic_id, 1 AS item_id, 1 AS specialization_id, 'Base Questions' AS name, 1 AS ordinal, 't'::BOOLEAN AS modified, NULL AS parent
          UNION
          SELECT 0, 1, 4396, 'Base SEAS Questions', 2, 't', NULL
          UNION
          SELECT 0, 1, 3736, 'Base Engineering Questions', 3, 't', NULL
          UNION
          SELECT 0, 1, 3733, 'Base Bioengineering Questions', 4, 't', NULL
          UNION
          SELECT 0, 1, 3111, 'Base Chemistry Questions', 5, 't', NULL                 
          UNION
          SELECT 0, 1456, 24450, 'Base Biology Lecture Questions', 6, 't', NULL                 
          UNION
          SELECT 0, 1483, 24450, 'Base Biology Lab Questions', 7, 't', NULL                 
          UNION 
          SELECT 0, 1627, 25134, 'Business School Questions', 8, 't', NULL                 
         
        ) AS t ORDER BY ordinal
      ",
      TOPICS_LITERAL
    ),

    array
    ( "<h5>Course Surveys</h5>\n",
      "
        SELECT t.topic_id, t.item_id, t.specialization_id, get_course(t.course_id) AS name, specialization_modified(t.item_id, t.specialization_id) AS modified
        FROM wces_course_topics AS t
        ORDER BY name
      ", 
      TOPICS_COURSE
    ),
    
    array
    ( "<h5>Class Section Surveys</h5>\n",
      "
        SELECT t.topic_id, t.item_id, t.specialization_id, get_class(t.class_id) AS name, specialization_modified(t.item_id, t.specialization_id) AS modified, t.category_id
        FROM wces_topics AS t
        WHERE
        " . ($question_period_id
             ? "(t.question_period_id = $question_period_id)" 
             : "(0 = 1)") . "
        ORDER BY name
      ", 
      TOPICS_CLASS
    )
  );
}

?>
