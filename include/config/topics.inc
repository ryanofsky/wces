<?

// get constants
require_once('wces/TopicEditor.inc');

function topics_list($question_period_id)
{
  return array
  (  // heading, sql, name type
    array
    ( '', 
      "
        SELECT * FROM
        (
          SELECT 1 AS item_id, 1 AS specialization_id, 'Base Questions' AS name, 1 AS ordinal, 't'::BOOLEAN AS modified, NULL AS parent
          UNION
          SELECT 1, 4396, 'Base SEAS Questions', 2, 't', NULL
          UNION
          SELECT 1, 3736, 'Base Engineering Questions', 3, 't', NULL
          UNION
          SELECT 1, 3733, 'Base Bioengineering Questions', 4, 't', NULL
          UNION
          SELECT 1, 3490, 'Gateway Lab Questions', 5, 't', NULL        
        ) AS t ORDER BY ordinal
      ",
      TOPICS_LITERAL
    ),
    
    array
    ( "<h5>Individual Classes</h5>\n",
      "
        SELECT t.item_id, t.specialization_id, get_class(t.class_id) AS name, specialization_modified(t.item_id, t.specialization_id) AS modified, t.category_id
        FROM wces_topics AS t
        WHERE t.question_period_id = $question_period_id
        GROUP BY t.specialization_id, t.item_id, t.class_id, t.category_id
        ORDER BY name
      ", 
      TOPICS_CLASS
    )
  );
}

?>