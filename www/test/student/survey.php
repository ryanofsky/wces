<?
  require_once("wces/general.inc");
  require_once("test/test.inc");

  param($classid);
  test_top("Survey","student",$classid);
  
  $name = $classid == 1 ? "COMS3210 Discrete Math - Professor Jonathan Gross" : "COMS3156 Software Engineering - Professor Gail Kaiser";
  
?>

<h2><?=$name?></h2>

<form method=post>
<input type=hidden name="preview_form_isstale" value="1">
<input type=hidden name="preview_action__" value="0"><input type=hidden name="preview_tas" value="a:0:{}">
<h4>General Questions</h4>
<p><b>Instructor: Organization and Preparation</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1a" value="a" ><label for="preview_Q1_MC1a">excellent</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1b" value="b" ><label for="preview_Q1_MC1b">very good</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1c" value="c" ><label for="preview_Q1_MC1c">satisfactory</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1d" value="d" ><label for="preview_Q1_MC1d">poor</label>
  <input type=radio name="preview_Q1_MC1" id="preview_Q1_MC1e" value="e" ><label for="preview_Q1_MC1e">disastrous</label>
</blockquote>
<p><b>Instructor: Classroom Delivery</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2a" value="a" ><label for="preview_Q1_MC2a">excellent</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2b" value="b" ><label for="preview_Q1_MC2b">very good</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2c" value="c" ><label for="preview_Q1_MC2c">satisfactory</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2d" value="d" ><label for="preview_Q1_MC2d">poor</label>
  <input type=radio name="preview_Q1_MC2" id="preview_Q1_MC2e" value="e" ><label for="preview_Q1_MC2e">disastrous</label>
</blockquote>
<p><b>Instructor: Approachability</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3a" value="a" ><label for="preview_Q1_MC3a">excellent</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3b" value="b" ><label for="preview_Q1_MC3b">very good</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3c" value="c" ><label for="preview_Q1_MC3c">satisfactory</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3d" value="d" ><label for="preview_Q1_MC3d">poor</label>
  <input type=radio name="preview_Q1_MC3" id="preview_Q1_MC3e" value="e" ><label for="preview_Q1_MC3e">disastrous</label>
</blockquote>
<p><b>Instructor: Overall Quality</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC4" id="preview_Q1_MC4a" value="a" ><label for="preview_Q1_MC4a">excellent</label>
  <input type=radio name="preview_Q1_MC4" id="preview_Q1_MC4b" value="b" ><label for="preview_Q1_MC4b">very good</label>
  <input type=radio name="preview_Q1_MC4" id="preview_Q1_MC4c" value="c" ><label for="preview_Q1_MC4c">satisfactory</label>
  <input type=radio name="preview_Q1_MC4" id="preview_Q1_MC4d" value="d" ><label for="preview_Q1_MC4d">poor</label>
  <input type=radio name="preview_Q1_MC4" id="preview_Q1_MC4e" value="e" ><label for="preview_Q1_MC4e">disastrous</label>
</blockquote>
<p><b>Course: Amount Learned</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC5" id="preview_Q1_MC5a" value="a" ><label for="preview_Q1_MC5a">excellent</label>
  <input type=radio name="preview_Q1_MC5" id="preview_Q1_MC5b" value="b" ><label for="preview_Q1_MC5b">very good</label>
  <input type=radio name="preview_Q1_MC5" id="preview_Q1_MC5c" value="c" ><label for="preview_Q1_MC5c">satisfactory</label>
  <input type=radio name="preview_Q1_MC5" id="preview_Q1_MC5d" value="d" ><label for="preview_Q1_MC5d">poor</label>
  <input type=radio name="preview_Q1_MC5" id="preview_Q1_MC5e" value="e" ><label for="preview_Q1_MC5e">disastrous</label>
</blockquote>
<p><b>Course: Appropriateness of Workload</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC6" id="preview_Q1_MC6a" value="a" ><label for="preview_Q1_MC6a">excellent</label>
  <input type=radio name="preview_Q1_MC6" id="preview_Q1_MC6b" value="b" ><label for="preview_Q1_MC6b">very good</label>
  <input type=radio name="preview_Q1_MC6" id="preview_Q1_MC6c" value="c" ><label for="preview_Q1_MC6c">satisfactory</label>
  <input type=radio name="preview_Q1_MC6" id="preview_Q1_MC6d" value="d" ><label for="preview_Q1_MC6d">poor</label>
  <input type=radio name="preview_Q1_MC6" id="preview_Q1_MC6e" value="e" ><label for="preview_Q1_MC6e">disastrous</label>
</blockquote>
<p><b>Course: Fairness of Grading Process</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC7" id="preview_Q1_MC7a" value="a" ><label for="preview_Q1_MC7a">excellent</label>
  <input type=radio name="preview_Q1_MC7" id="preview_Q1_MC7b" value="b" ><label for="preview_Q1_MC7b">very good</label>
  <input type=radio name="preview_Q1_MC7" id="preview_Q1_MC7c" value="c" ><label for="preview_Q1_MC7c">satisfactory</label>
  <input type=radio name="preview_Q1_MC7" id="preview_Q1_MC7d" value="d" ><label for="preview_Q1_MC7d">poor</label>
  <input type=radio name="preview_Q1_MC7" id="preview_Q1_MC7e" value="e" ><label for="preview_Q1_MC7e">disastrous</label>
</blockquote>
<p><b>Course: Quality of Text</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC8" id="preview_Q1_MC8a" value="a" ><label for="preview_Q1_MC8a">excellent</label>
  <input type=radio name="preview_Q1_MC8" id="preview_Q1_MC8b" value="b" ><label for="preview_Q1_MC8b">very good</label>
  <input type=radio name="preview_Q1_MC8" id="preview_Q1_MC8c" value="c" ><label for="preview_Q1_MC8c">satisfactory</label>
  <input type=radio name="preview_Q1_MC8" id="preview_Q1_MC8d" value="d" ><label for="preview_Q1_MC8d">poor</label>
  <input type=radio name="preview_Q1_MC8" id="preview_Q1_MC8e" value="e" ><label for="preview_Q1_MC8e">disastrous</label>
</blockquote>
<p><b>Course: Overall Quality</b></p>
<blockquote>
  <input type=radio name="preview_Q1_MC10" id="preview_Q1_MC10a" value="a" ><label for="preview_Q1_MC10a">excellent</label>
  <input type=radio name="preview_Q1_MC10" id="preview_Q1_MC10b" value="b" ><label for="preview_Q1_MC10b">very good</label>
  <input type=radio name="preview_Q1_MC10" id="preview_Q1_MC10c" value="c" ><label for="preview_Q1_MC10c">satisfactory</label>
  <input type=radio name="preview_Q1_MC10" id="preview_Q1_MC10d" value="d" ><label for="preview_Q1_MC10d">poor</label>
  <input type=radio name="preview_Q1_MC10" id="preview_Q1_MC10e" value="e" ><label for="preview_Q1_MC10e">disastrous</label>
</blockquote>
<p><b>Comments</b></p>
<blockquote><textarea name="preview_Q1_FR1" rows=8 cols=50 wrap=virtual></textarea></blockquote>
<h4>Custom Questions from Professor Test Professor</h4>
<p><b>This is test.</b></p>
<blockquote>
  <input type=radio name="preview_Q153_MC1" id="preview_Q153_MC1a" value="a" ><label for="preview_Q153_MC1a">excellent</label>
  <input type=radio name="preview_Q153_MC1" id="preview_Q153_MC1b" value="b" ><label for="preview_Q153_MC1b">very good</label>
  <input type=radio name="preview_Q153_MC1" id="preview_Q153_MC1c" value="c" ><label for="preview_Q153_MC1c">satisfactory</label>
  <input type=radio name="preview_Q153_MC1" id="preview_Q153_MC1d" value="d" ><label for="preview_Q153_MC1d">poor</label>
  <input type=radio name="preview_Q153_MC1" id="preview_Q153_MC1e" value="e" ><label for="preview_Q153_MC1e">disastrous</label>
</blockquote>
<h4>ABET Questions</h4>
<table bordercolor=black cellspacing=0 cellpadding=3 border=0 RULES="groups" FRAME=box STYLE="border: none">
<thead>
<tr>
<td colspan=2 align=left STYLE="border: none"><b>To what degree did this course enhance your ability to ...</b></td>
<td colspan=9 align=right STYLE="border: none"><b>(0 = <i>not at all</i>, 5 = <i>a great deal</i>)</b></td>
</tr>
</thead>
<tbody>
<tr>
  <td bgcolor=black background="/wces/media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="/wces/media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="/wces/media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="/wces/media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="/wces/media/0x000000.gif">&nbsp;</td>
  <td bgcolor=black background="/wces/media/0x000000.gif" align=center><font color=white><STRONG>0</STRONG></font></td>
  <td bgcolor=black background="/wces/media/0x000000.gif" align=center><font color=white><STRONG>1</STRONG></font></td>
  <td bgcolor=black background="/wces/media/0x000000.gif" align=center><font color=white><STRONG>2</STRONG></font></td>
  <td bgcolor=black background="/wces/media/0x000000.gif" align=center><font color=white><STRONG>3</STRONG></font></td>
  <td bgcolor=black background="/wces/media/0x000000.gif" align=center><font color=white><STRONG>4</STRONG></font></td>
  <td bgcolor=black background="/wces/media/0x000000.gif" align=center><font color=white><STRONG>5</STRONG></font></td>
</tr>
<tr><td colspan=5 bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif">Design experiments</td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET1" value="a" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET1" value="b" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET1" value="c" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET1" value="d" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET1" value="e" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET1" value="f" type=radio></td></tr>
<tr><td colspan=5 bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif">Solve engineering problems</td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET8" value="a" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET8" value="b" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET8" value="c" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET8" value="d" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET8" value="e" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET8" value="f" type=radio></td></tr>
<tr><td colspan=5 bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif">Communicate using oral presentations</td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET12" value="a" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET12" value="b" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET12" value="c" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET12" value="d" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET12" value="e" type=radio></td><td align=center bgcolor="#EEEEEE" background="/wces/media/0xEEEEEE.gif"><input name="preview_Q153_ABET12" value="f" type=radio></td></tr>
<tr><td colspan=5 bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif">Communicate using written reports</td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET13" value="a" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET13" value="b" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET13" value="c" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET13" value="d" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET13" value="e" type=radio></td><td align=center bgcolor="#FFFFFF" background="/wces/media/0xFFFFFF.gif"><input name="preview_Q153_ABET13" value="f" type=radio></td></tr>

</tbody>
</table>
<h4>TA Ratings</h4><p>If your class has teaching assistants, you can use this section of the survey to rate them.</p><input type=submit name="preview_action_3_new" value="Rate a TA..."><p>&nbsp;</p>
<input type=submit name=submit value="Save these responses">
<input type=submit name=submit value="Return to class list without saving">
</form>

<? test_bottom(); ?>