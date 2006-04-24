<HTML>
<HEAD>
 <TITLE>NGWinamp : Anonymous PHP Remote Control Client</TITLE>
<STYLE TYPE="text/css">
BODY {font-family:courier new;font-size:12px;color:black;}
.tiny {font-size:10px;}
.bold {font-weight:bold;}
.italic {font-style:italic;}
.title {font-size:20px;}
.info {font-style:italic;}
.error {font-style:italic;font-weight:bold;color:red;}
.link {text-decoration:none;color:black;}
.link:hover {text-decoration:underline;font-weight:bold;color:blue;}
.cursong {background-color:black;color:white;}
.normalsong {background-color:white;color:black;}
</STYLE>
</HEAD>

<BODY>
<?php

/********************************************************/
/* NGWinamp Remote Control for PHP						*/
/********************************************************/
/* 														*/
/* Author  : Antony Ducommun							*/
/* Email   : nitro@webmails.com							*/
/* Date    : 10/11/2004									*/
/* Version : V1.1.0										*/
/* 														*/
/* Made for NGWinamp V1.0.3 and higher.					*/
/* Best viewing with tabulation length to 4.			*/
/* 														*/
/* This is an EXEMPLE of what is possible to do with	*/
/* PHP class NGWINAMPCLIENT. For more detail, go to the */
/* "ngwinamp.class.php" file.							*/
/* 														*/
/********************************************************/

// Loading global variable
function loadgvar($name, $default) {
	global $_REQUEST;

	if (isset($_REQUEST[$name]) == FALSE) {
		return $default;
	}
	return $_REQUEST[$name];
}


// NGWinamp Client Interface (instance $ngwc)
include("ngwinamp.class.php");
$ngwc = new NGWINAMPCLIENT;

// Loading parameters
$host = loadgvar("host", "");
$action = loadgvar("action", "");
$pl_op = loadgvar("pl_op", "");
$bw_path = loadgvar("bw_path", "");
$bw_addfile = loadgvar("bw_addfile", "");
$bw_op = loadgvar("bw_op", "");

// Default parameters
if(strlen($host) == 0)
	$host = "localhost";
if(strlen($bw_path) > 0)
{
	$bw_path = stripslashes($bw_path);

	if($bw_path[strlen($bw_path) - 1] == '\\')
	{
		$bw_path = substr($bw_path, 0, strlen($bw_path) - 1);
	}
	if($bw_path[0] == '\\' && $bw_path[1] == '\\')
	{
		$bw_path = str_replace("\\\\", "\\", $bw_path);
		$bw_path = "\\".$bw_path;
	}
	else
		$bw_path = str_replace("\\\\", "\\", $bw_path);
}

// ensure a link is good formatted
function nglink($value)
{
	$value = str_replace(" ", "%20", $value);
	$value = str_replace("&", "%26", $value);
	$value = str_replace("#", "%23", $value);
	$value = str_replace("+", "%2B", $value);
	if($value[0] == '\\' && $value[1] == '\\')
	{
		$value = str_replace("\\\\", "\\", $value);
		$value = "\\".$value;
	}
	else
		$value = str_replace("\\\\", "\\", $value);
	if($value[strlen($value) - 1] == '\\')
		$value = substr($value, 0, strlen($value) - 1);
	return $value;
}

// write some space to ident directories
function ngwriteident($value)
{
	for($i = 0; $i < $value; $i++)
		echo("&nbsp;");
}

// list a path and it's sub paths until it reach maxcount
function nglistpath($ngwc, $path)
{
	global $host, $bw_path;

	$count = 0;
	$linkpath = nglink($path);
	$bw_list = $ngwc->bw_getlist($path);
	if(count($bw_list['directories']) > 0)
	{
		for($j = 0; $j < count($bw_list['directories']); $j++)
		{
			ngwriteident($ident + 1);
			$newpath = "$path\\".$bw_list['directories'][$j];
			$newlinkpath = nglink($newpath);
			echo("<A CLASS=\"bold italic link\" HREF=\"?host=$host&action=browse&bw_op=list&bw_path=$newlinkpath\">$newpath</A> - [<A CLASS=\"bold italic link\" HREF=\"?host=$host&action=browse&bw_op=add&bw_addfile=$newpath&bw_path=$newpath\">add</A>]<BR>\n");
			$count++;
		}
		echo("<BR>\n");
		$bw_list['directories'] = array();
	}
	if(count($bw_list['files']) > 0)
	{
		for($j = 0; $j < count($bw_list['files']); $j++)
		{
			ngwriteident($ident + 3);
			$linkfile = nglink($path."\\".$bw_list['files'][$j]);
			$curpath = nglink($bw_path);
			echo("<A CLASS=\"bold italic link\" HREF=\"?host=$host&action=browse&bw_op=add&bw_addfile=$linkfile&bw_path=$curpath\">{$bw_list['files'][$j]}</A><BR>\n");
			$count++;
		}
		echo("<BR>\n");
		$bw_list['files'] = array();
	}
	echo("<BR>\n");
	
	return $count;
}

$showauth = FALSE;

// connect to NGWinamp server if possible
if($ngwc->connect($host))
{
	// authenticate with server
	if($ngwc->authenticate("") == NGWINAMP_AUTH_SUCCESS)
	{
		// OK : success
		echo("<P CLASS=\"info\">Connected... sending requests...</P>\n");

		// Get stats
		$sn_volume = sprintf("%.01f", $ngwc->sn_getvolume());
		$sn_pan = sprintf("%.01f", $ngwc->sn_getpan());
		$tmp = $ngwc->sn_getpos();
		$sn_pos = sprintf("%.01f", $tmp['pos']);
		$sn_postime = sprintf("%.01f", $tmp['posms'] / 1000.0);
		$sn_length = sprintf("%.01f", $tmp['lengthms'] / 1000.0);
		 // retrieve playlist infos
		$tmp = $ngwc->pl_getpos();
		$pl_pos = $tmp['index'];
		$pl_length = $tmp['length'];
		if($ngwc->pl_getshuffle())
			$pl_shuffle = "enabled";
		else
			$pl_shuffle = "disabled";
		if($ngwc->pl_getrepeat())
			$pl_repeat = "enabled";
		else
			$pl_repeat = "disabled";
		 // retrieve current song name
		$pl_curname = $ngwc->pl_getnames($pl_pos);
		$pl_curname = $pl_curname[0];
		$pl_curfile = $ngwc->pl_getfiles($pl_pos);
		$pl_curfile = $pl_curfile[0];

		// Output infos
?>
<P>&nbsp;</P>
<P CLASS="bold title">INFOS</P>
<P>
playing name : <SPAN CLASS="bold italic"><?php echo($pl_curname); ?></SPAN><BR>
playing file : <SPAN CLASS="bold italic"><?php echo($pl_curfile); ?></SPAN><BR>
song status &nbsp;: <SPAN CLASS="bold italic"><?php echo("$sn_pos% ($sn_postime / $sn_length [s])"); ?></SPAN><BR>
position <?php ngwriteident(4); ?>: <SPAN CLASS="bold italic"><?php echo(($pl_pos + 1)." / $pl_length"); ?><BR></SPAN><BR>
volume &nbsp;: <SPAN CLASS="bold italic"><?php echo($sn_volume); ?> %</SPAN><BR>
pan <?php ngwriteident(4); ?>: <SPAN CLASS="bold italic"><?php echo($sn_pan); ?> %</SPAN><BR>
shuffle : <SPAN CLASS="bold italic"><?php echo($pl_shuffle); ?></SPAN><BR>
repeat &nbsp;: <SPAN CLASS="bold italic"><?php echo($pl_repeat); ?></SPAN><BR>
</P>
<?php

		if(strcmp($action, "playlist") == 0)
		{
			if(strcmp($pl_op, "view") == 0)
			{

?>
<P>&nbsp;</P>
<P CLASS="bold title">CURRENT PLAYLIST ITEMS</P>
<P>
<TABLE BORDER="0" CELLSPACING="5" CELLPADDING="2">
<?php
				// retrieve playlist item's name
				$pl_names = $ngwc->pl_getnames();
				$pl_files = $ngwc->pl_getfiles();
				for($i = count($pl_names) - 1; $i >= 0; $i--)
				{
					$id = $i + 1;
					$name = $pl_names[$i];
					$file = $pl_files[$i];
					if($i == $pl_pos)
						$curstyle = " cursong";
					else
						$curstyle = " normalsong";
						
?>
 <TR>
  <TD WIDTH="1%" CLASS="italic<?php echo($curstyle); ?>" ALIGN="center" VALIGN="center" HEIGHT="40"><?php echo($id); ?></TD>
  <TD WIDTH="49%" CLASS="bold italic<?php echo($curstyle); ?>" ALIGN="left" VALIGN="center" HEIGHT="40"><?php echo($name); ?></TD>
  <TD WIDTH="50%" CLASS="tiny<?php echo($curstyle); ?>" ALIGN="left" VALIGN="center" HEIGHT="40"><?php echo($file); ?></TD>
<?php
?>
  </TD>
 </TR>
<?php
				}
			}
			echo("</TABLE>\n</P>\n");
		}

		if(strcmp($action, "browse") == 0)
		{
			if(strcmp($bw_op, "add") == 0)
			{
				$ngwc->pl_addfiles(array(stripslashes($bw_addfile)));
				$bw_op = "list";
			}
			if(strcmp($bw_op, "roots") == 0)
			{
?>
<P>&nbsp;</P>
<P CLASS="bold title">BROWSE-ROOTS</P>
<P>
<?php
				$bw_roots = $ngwc->bw_getroots();
				for($i = 0; $i < count($bw_roots); $i++)
				{
					$linkpath = nglink($bw_roots[$i]);
					echo("<A CLASS=\"italic bold link\" HREF=\"?host=$host&action=browse&bw_op=list&bw_path=$linkpath\">{$bw_roots[$i]}</A><BR>\n");
				}
				echo("</P><P CLASS=\"info\">count : ".count($bw_roots)."</P>\n");
				echo("</P>\n");
			}
			if(strcmp($bw_op, "list") == 0)
			{
?>
<P>&nbsp;</P>
<P CLASS="bold title">BROWSE-LIST</P>
<P>
<?php
				$uppath = substr($bw_path, 0, strrpos($bw_path, '\\'));
				$uplinkpath = nglink($uppath);
				echo("<SPAN CLASS=\"info\">go up&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: <A CLASS=\"bold italic link\" HREF=\"?host=$host&action=browse&bw_op=list&bw_path=$uplinkpath\">$uppath</A></SPAN><BR>\n");
	
				$curlinkpath = nglink($bw_path);
				$maxcount = $bw_maxcount;
				$newcount = $maxcount + 50;
				echo("<SPAN CLASS=\"info\">list from : <A CLASS=\"bold italic link\" HREF=\"?host=$host&action=browse&bw_op=list&bw_path=$curlinkpath\">$bw_path</A></SPAN><BR><BR>\n");
				$c = nglistpath($ngwc, $bw_path);
				echo("</P><P CLASS=\"italic info\">count : $c</P>\n");
			}
			echo("</P>\n");
		}

		// Output commands
?>
<P>&nbsp;</P>
<P CLASS="bold title">COMMANDS</P>
<P>
<FORM METHOD="POST">
 <INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
 <INPUT TYPE="hidden" NAME="action" VALUE="playlist">
 <INPUT TYPE="hidden" NAME="pl_op" VALUE="view">
 <INPUT TYPE="submit" VALUE="view playlist">
</FORM>
</P>
<P>
<FORM METHOD="POST">
 <INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
 <INPUT TYPE="hidden" NAME="action" VALUE="browse">
 <INPUT TYPE="hidden" NAME="bw_op" VALUE="roots">
 <INPUT TYPE="submit" VALUE="view roots">
</FORM>
</P>
<?php
		if(strcmp($action, "disconnect") != 0)
		{
?>
<P>
<FORM METHOD="POST">
 <INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
 <INPUT TYPE="hidden" NAME="action" VALUE="disconnect">
 <INPUT TYPE="submit" VALUE="disconnect">
</FORM>
</P>
<?php
		}
	}
	else
	{
		// cannot authenticate. in this case the connection properties are shown
		echo("<P CLASS=\"error\">Cannot authenticate !</P>\n");
		$showauth = TRUE;
	}
}
else
{
	// cannot connect. in this case the connection properties are shown
	echo("<P CLASS=\"error\">Cannot connect !</P>\n");
	$showauth = TRUE;
}
if(strcmp($action, "disconnect") == 0)
	$showauth = true;
if($showauth)
{
?>
<P>&nbsp;</P>
<P CLASS="bold title">CONNECTION PARAMS</P>
<P>
<FORM METHOD="POST">
host &nbsp;&nbsp;: <INPUT TYPE="text" NAME="host" VALUE="<?php echo($host); ?>" SIZE="10"><BR><BR>
<INPUT TYPE="submit" VALUE="connect" SIZE="10">
</FORM>
</P>
<?php
}

?>
</BODY>
</HTML>
