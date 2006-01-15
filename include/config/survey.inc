<?

require_once('wbes/component_choice.inc');
require_once('wbes/component_heading.inc');
require_once('wbes/component_pagebreak.inc');
require_once('wbes/component_textresponse.inc');
require_once('wbes/component_text.inc');

if ($WCES_COLUMBIA)
{
  require_once('wces/component_abet.inc');
  require_once('wces/component_instructor.inc');
  require_once('wces/component_ethics.inc');
}

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

if ($WCES_COLUMBIA)
{
  array_append
  (
    $WCES_CONFIG_FACTORIES,
    array
    (
      new AbetFactory(),
      new NewAbetFactory(),
      new BioAbetFactory(),
      new InstructorSurveyFactory(),
      new EthicsSurveyFactory()
    )
  );
}

?>
