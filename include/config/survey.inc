<?

require_once('wbes/component_choice.inc');
require_once('wbes/component_heading.inc');
require_once('wbes/component_pagebreak.inc');
require_once('wbes/component_textresponse.inc');
require_once('wbes/component_text.inc');
require_once('wces/component_abet.inc');

$survey_factories = array
( 
  new SurveyFactory(),
  new AbetFactory(),
  new NewAbetFactory(),
  new BioAbetFactory(),
  new ChoiceFactory(),
  new ChoiceQuestionFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory(),
  new PageBreakFactory()
);

?>
