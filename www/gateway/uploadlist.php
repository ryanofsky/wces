<?

require_once("wces/wces.inc");
require_once("wbes/postgres.inc");

$profs = array(35,6202);

$classes = array
( 25773 => array
  ( 'ael2006', 'cl2140', 'cjl2018', 'vrm2003', 'tvm2001', 'tsm2004', 'lbm2017',
    'fm2055', 'aom2006', 'jhm2030', 'jkm2017', 'rjn2003', 'ppo2004', 'wrp2003',
    'ar2147', 'bpr2006', 'es2046', 'dws2005', 'ais2009', 'vs2042', 'ss2263',
    'gts2006', 'jds2019', 'as2253', 'jms2084', 'jju2002', 'rv2041', 'jlw2028',
    'mw2133', 'jyw2003', 'ww2031', 'ayx1', 'afy2002', 'jcz2002',
    'xz2025'
  ),

  25774 => array
  ( 'sbe2007', 'rql1', 'fm2054', 'mcm2032', 'sam2044', 'mbm2044', 'svm2004',
    'akm2010', 'pep2007', 'aap2003', 'agp2014', 'fap2005', 'ejp2016', 'ar2149',
    'ryr2001', 'awr2011', 'mks2017', 'jss2050', 'dps2013', 'es2156', 'mjs2054',
    'rhs2027', 'ws2012', 'jms2085', 'pjs2018', 'rt2055', 'rev2001', 'vsv2003',
    'caw2034', 'akw2006', 'dy2018', 'jy2042', 'ny2009',
    'cdz2003'
  ),

  25775 => array
  ( 'eja2001', 'jml2006', 'yl2084', 'hjl2008', 'ejl2011', 'eml2023', 'enm2007',
    'jtm2014', 'wfm2004', 'kam2039', 'jjm2052', 'jsp2014', 'dar2018', 'res2049',
    'kls2015', 'sds2018', 'mss2049', 'hs2113', 'jrs2046', 'ocs2004', 'mss2047',
    'sls2039', 'cis2005', 'tt2046', 'ot2006', 'nt2039', 'krt2005', 'vvv2001',
    'mjv2008', 'dv2027', 'azw2001', 'dw2071', 'vww2003', 'pjy2001', 'ifz2001',
    'cjz2002'
  ),

  25776 => array
  ( 'ejl2010', 'jyl2010', 'jjm2053', 'smm2042', 'rhm2022', 'vqn1', 'wp2018',
    'lmp2017', 'cp2093', 'dp2067', 'mp2151', 'pjp2014', 'rtp2005', 'ar2151',
    'qr2003', 'pr2045', 'msr2026', 'dms2036', 'aas2044', 'shs2019', 'bfs2018',
    'ss2261', 'ayt2002', 'att2005', 'dtw2004', 'hw2047', 'sfw2003', 'gew2005',
    'bnw2001', 'sww2004', 'lry2002', 'acy7'
  ),

  25777 => array
  ( 'rgc2008', 'pcl2007', 'xl2017', 'sml2029', 'jhm2031', 'yp2026', 'asp2019',
    'sp2146', 'ap2118', 'ar2150', 'is2041', 'scs2017', 'rs2177', 'jns2011',
    'ths2007', 'gss2002', 'acs2031', 'rjs2040', 'tjs2012', 'lt2045', 'kct2006',
    'slt2013', 'tjw2005', 'uaw2', 'iw2019', 'jbw2017', 'kky2003', 'pky2003',
    'afy2003', 'gsy2001', 'wy2004'
  )
);

$sql = '';
foreach ($classes as $class_id => $students)
{
  $sql .= "DELETE FROM enrollments WHERE class_id = $class_id AND status = 3;\n";
  $sql .= "UPDATE enrollments SET status = 0 WHERE class_id = $class_id AND status = 1;\n";


  foreach($students as $cunix)
  {
    $student = "user_update('" . addslashes($cunix) . "', null, null, null, 8, null, null)";
    $sql .= "SELECT enrollment_update($student, $class_id, 1, 'now');\n";
  }

  foreach($profs as $prof)
  {
    $sql .= "SELECT enrollment_update($prof, $class_id, 3, 'now');\n";
  }
}

$server_isproduction = false;
$db_debug = true;

wces_connect();
$result = pg_go("BEGIN;\n$sql\nROLLBACK;", $wces, __FILE__, __LINE__);
pg_show($result);
print("result = $result");


?>
