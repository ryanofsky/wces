<%

require_once("page.inc");
require_once("questions.inc");

define("QuestionEditor_INSERT",1);
define("QuestionEditor_EDIT",2);
define("QuestionEditor_MOVEUP",3);
define("QuestionEditor_MOVEDOWN",4);
define("QuestionEditor_DELETE",4);

class QuestionEditor extends Widget
{
  var action;
  var questionset;
  
  function QuestionEditor($prefix,$form,$formmethod)
  {
    Widget($prefix,$form,$formmethod);
    $this->action = new ActionButton($prefix . "_action", $form, $formmethod);
  }
  
  function loadvalues()
  {      
  }
  
  function display($hidden)
  {
    if($this->action->action == "insert")
    {
    
    
    }
    else
    { 
%>
<p>Welcome to the question set editor.</p>
<table rules=all frame=void>
<% foreach($this->questionset->questions as $key => $question) { %>
  <tr>
    <td bgcolor="#DDDDDD">&nsbp;</td>
    <td><center><%$this->action->display(QuestionEditor_INSERT,$key,"Insert New Component");%></center></td>
  </tr>
  <tr>
    <td bgcolor="#DDDDDD">&nbsp;</td>
    <td>
      <table>
        <tr>
          <td><%$this->action->display(QuestionEditor_EDIT,$key,"Edit");%></td>
          <td><%$this->action->display(QuestionEditor_MOVEUP,0,"Move Up"%></td>
        </tr>
        <tr>
          <td><%$this->action->display(QuestionEditor_DELETE,$key,"Delete")%></td>
          <td><%$this->action->display(QuestionEditor_MOVEUPDOWN,$key,"Move Down"%></td>
        </tr>
      </table>
    </td>
    <td>
      <% $i = $question->getwidget($this->prefix . "_0,"$this->form,$this->formmethod); %>
      <% $i->display %>
    </td>
  </tr>
<% } %>
  <tr>
    <td bgcolor="#DDDDDD">&nsbp;</td>
    <td><center><%$this->action->display("insert",$key+1,"Insert New Component");%></center></td>
  </tr>
</table>
<%
    }  
  }
  
  function loadquestionset($db,$qid)
  {
    $y = mysql_query("SELECT qdata FROM NewQuestionSets WHERE questionsetid = '$qid'",$db);
    $n = mysql_fetch_array($y);
    $this->questionset = unserialize($n["qdata"]);
    
  }
};

page_top("Question Editor");

$qs = new QuestionEditor("qe","f",WIDGET_POST);

page_bottom()


%>