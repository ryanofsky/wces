<?

require_once('wbes/component_choice.inc');
require_once('wbes/component_heading.inc');
require_once('wbes/component_pagebreak.inc');
require_once('wbes/component_textresponse.inc');
require_once('wbes/component_text.inc');

$WCES_CONFIG_FACTORIES = array
( 
  new SurveyFactory(),
  new ChoiceFactory(),
  new ChoiceQuestionFactory(),
  new TextResponseFactory(),
  new TextFactory(),
  new HeadingFactory(),
  new PageBreakFactory()
);

?>