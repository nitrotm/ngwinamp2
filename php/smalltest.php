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


// browsing variable
$curcount = 0;
$maxcount = 100;
$host = loadgvar("host", "");
$passwd = loadgvar("passwd", "");
$action = loadgvar("action", "");
$sn_op = loadgvar("sn_op", "");
$sn_volume = loadgvar("sn_volume", "");
$sn_pan = loadgvar("sn_pan", "");
$sn_pos = loadgvar("sn_pos", "");
$pl_op = loadgvar("pl_op", "");
$bw_op = loadgvar("bw_op", "");
$bw_path = loadgvar("bw_path", "");
$bw_addfile = loadgvar("bw_addfile", "");
$bw_maxcount = loadgvar("bw_maxcount", 0);

// Default parameters
if(strlen($host) == 0)
	$host = "localhost";
if(strlen($passwd) == 0)
	$passwd = "abcdef";
if(strlen($bw_maxcount) == 0)
	$bw_maxcount = $maxcount;
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

// write some space to ident directories
function ngwriteident($value)
{
	for($i = 0; $i < $value; $i++)
		echo("&nbsp;");
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

// list a path and it's sub paths until it reach maxcount
function nglistpath($ngwc, $path, $ident)
{
	global $host, $passwd, $bw_path;
	global $curcount, $maxcount;

	$linkpath = nglink($path);
	$recurse = FALSE;
	$bw_list = $ngwc->bw_getlist($path);
	if(count($bw_list['files']) > 0)
	{
		$newcount = $maxcount + 50;
		if(($curcount + count($bw_list['directories'])) < $maxcount)
		{
			for($j = 0; $j < count($bw_list['files']); $j++)
			{
				if(($curcount + count($bw_list['directories'])) < $maxcount)
				{
					ngwriteident($ident + 3);
					$linkfile = nglink($path."\\".$bw_list['files'][$j]);
					$curpath = nglink($bw_path);
					echo("<A CLASS=\"link\" HREF=\"?host=$host&passwd=$passwd&action=browse&bw_op=add&bw_addfile=$linkfile&bw_path=$curpath&bw_maxcount=$maxcount\">{$bw_list['files'][$j]}</A><BR>\n");
					$curcount++;
				}
				else
				{
					ngwriteident($ident + 3);
					echo("more files ...<BR>\n");
					break;
				}
			}
			echo("<BR>\n");
			$bw_list['files'] = array();
		}
		else
		{
			ngwriteident($ident + 3);
			echo("more files ...<BR>\n");
		}
	}
	if(($curcount + count($bw_list['directories'])) < $maxcount)
	{
		$curcount += count($bw_list['directories']);
		$recurse = TRUE;
	}
	if(count($bw_list['directories']) > 0)
	{
		for($j = 0; $j < count($bw_list['directories']); $j++)
		{
			if($recurse || $curcount < $maxcount)
			{
				ngwriteident($ident + 1);
				$newpath = "$path\\".$bw_list['directories'][$j];
				$newlinkpath = nglink($newpath);
				echo("<A CLASS=\"link\" HREF=\"?host=$host&passwd=$passwd&action=browse&bw_op=view&bw_path=$newlinkpath&bw_maxcount=$maxcount\">$newpath</A><BR>\n");

				if($recurse)
				{
					nglistpath($ngwc, $path."\\".$bw_list['directories'][$j], $ident + 4);
					$recurse = TRUE;
				}
				else
					$curcount++;
			}
			else
			{
				ngwriteident($ident + 1);
				echo("more paths ...<BR>\n");
				break;
			}
		}
		echo("<BR>\n");
		$bw_list['directories'] = array();
	}
	echo("<BR>\n");
}


/************ SCRIPT BEGIN THERE ************/

?>
<HTML>
<HEAD>
 <TITLE>NGWinamp : PHP Remote Control Client</TITLE>
<STYLE TYPE="text/css">
BODY {font-family:courier new;font-size:12px;color:black;}
.title {font-size:20px;font-weight:bold;}
.info {font-style:italic;}
.error {font-style:italic;font-weight:bold;color:red;}
.link {text-decoration:none;color:black;}
.link:hover {text-decoration:underline;font-weight:bold;color:blue;}
</STYLE>
</HEAD>

<BODY>
<?php

// connect to NGWinamp server if possible
if($ngwc->connect($host))
{
	// authenticate with server
	if($ngwc->authenticate($passwd) == NGWINAMP_AUTH_SUCCESS)
	{
		// OK : success
		echo("<P CLASS=\"info\">Connected... sending requests...</P>\n");

		// sound requests
		if(strcmp($action, "sound") == 0)
		{
			if(strcmp($sn_op, "prev") == 0)
				$ngwc->sn_prev();
			if(strcmp($sn_op, "play") == 0)
				$ngwc->sn_play();
			if(strcmp($sn_op, "pause") == 0)
				$ngwc->sn_pause();
			if(strcmp($sn_op, "stop") == 0)
				$ngwc->sn_stop();
			if(strcmp($sn_op, "next") == 0)
				$ngwc->sn_next();

			if(strcmp($sn_op, "modify") == 0)
			{
				if(strlen($sn_volume) > 0)
					$ngwc->sn_setvolume($sn_volume);
				if(strlen($sn_pan) > 0)
					$ngwc->sn_setpan($sn_pan);
				if(strlen($sn_pos) > 0)
					$ngwc->sn_setpos($sn_pos);
			}
		}

		// playlist requests
		if(strcmp($action, "playlist") == 0)
		{
			if(strcmp($pl_op, "randomize") == 0)
				$ngwc->pl_randomize();
			if(strcmp($pl_op, "sort by path") == 0)
				$ngwc->pl_sortbypath();
			if(strcmp($pl_op, "sort by title") == 0)
				$ngwc->pl_sortbyname();
			if(strcmp($pl_op, "remove deads") == 0)
				$ngwc->pl_removedead();
			if(strcmp($pl_op, "clear") == 0)
				$ngwc->pl_clear();
		}

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

		// retrieve current sound infos
		$sn_volume = sprintf("%.01f", $ngwc->sn_getvolume());
		$sn_pan = sprintf("%.01f", $ngwc->sn_getpan());
		$tmp = $ngwc->sn_getpos();
		$sn_pos = sprintf("%.01f", $tmp['pos']);
		$sn_postime = sprintf("%.01f", $tmp['posms'] / 1000.0);
		$sn_length = sprintf("%.01f", $tmp['lengthms'] / 1000.0);



		/************ SCRIPT GUI BEGIN THERE ************/
?>
<HR>
<P CLASS="title">INFOS</P>
<P>
playing name : <?php echo($pl_curname); ?><BR>
playing file : <?php echo($pl_curfile); ?><BR>
song status &nbsp;: <?php echo("$sn_pos% ($sn_postime / $sn_length [s])"); ?><BR>
position <?php ngwriteident(4); ?>: <?php echo(($pl_pos + 1)." / $pl_length"); ?><BR><BR>
volume &nbsp;: <?php echo($sn_volume); ?> %<BR>
pan <?php ngwriteident(4); ?>: <?php echo($sn_pan); ?> %<BR>
shuffle : <?php echo($pl_shuffle); ?><BR>
repeat &nbsp;: <?php echo($pl_repeat); ?><BR>
</P>
<?php
	if(strcmp($action, "playlist") == 0 && strcmp($pl_op, "view") == 0)
	{
?>
<HR>
<P CLASS="title">PLAYLIST</P>
<P>
<?php
		// retrieve playlist item's name
		$pl_names = $ngwc->pl_getnames();
		$pl_files = $ngwc->pl_getfiles();
		for($i = 0; $i < count($pl_names); $i++)
			echo("item ".($i+1)." : {$pl_names[$i]}<BR>&nbsp;file : &lt;{$pl_files[$i]}><BR><BR>\n");
?>
</P>
<?php
	}
	if(strcmp($action, "browse") == 0)
	{
?>
<HR>
<P CLASS="title">BROWSE-LIST</P>
<P>
<?php
		if(strcmp($bw_op, "add") == 0)
			$ngwc->pl_addfiles(array(stripslashes($bw_addfile)));
		if(strcmp($bw_op, "get roots") == 0)
		{
			$bw_roots = $ngwc->bw_getroots();
			for($i = 0; $i < count($bw_roots); $i++)
			{
				$linkpath = nglink($bw_roots[$i]);
				echo("<A CLASS=\"link\" HREF=\"?host=$host&passwd=$passwd&action=browse&bw_op=view&bw_path=$linkpath&bw_maxcount=$maxcount\">{$bw_roots[$i]}</A><BR>\n");
			}
			echo("</P><P CLASS=\"info\">count : ".count($bw_roots)."</P>\n");
		}
		if(strcmp($bw_op, "view") == 0 || strcmp($bw_op, "add") == 0)
		{
			$uppath = substr($bw_path, 0, strrpos($bw_path, '\\'));
			$uplinkpath = nglink($uppath);
			echo("<SPAN CLASS=\"info\">go up&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;: <A CLASS=\"link\" HREF=\"?host=$host&passwd=$passwd&action=browse&bw_op=view&bw_path=$uplinkpath&bw_maxcount=$maxcount\">$uppath</A></SPAN><BR>\n");

			$curlinkpath = nglink($bw_path);
			$maxcount = $bw_maxcount;
			$newcount = $maxcount + 50;
			echo("<SPAN CLASS=\"info\">list from : <A CLASS=\"link\" HREF=\"?host=$host&passwd=$passwd&action=browse&bw_op=view&bw_path=$curlinkpath&bw_maxcount=$newcount\">$bw_path</A></SPAN><BR><BR>\n");
			nglistpath($ngwc, $bw_path, 0);
			echo("</P><P CLASS=\"info\">count : $curcount</P>\n");
		}
	}
?>
<HR>
<P CLASS="title">MISC COMMANDS</P>
<P>
<FORM METHOD="POST">
<INPUT TYPE="hidden" NAME="action" VALUE="sound">
<INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
<INPUT TYPE="submit" NAME="sn_op" VALUE="reload" SIZE="10">
</FORM>
</P>
<HR>
<P CLASS="title">SOUND COMMANDS</P>
<P>
<FORM METHOD="POST">
<INPUT TYPE="hidden" NAME="action" VALUE="sound">
<INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
<INPUT TYPE="hidden" NAME="passwd" VALUE="<?php echo($passwd); ?>">
<INPUT TYPE="submit" NAME="sn_op" VALUE="prev" SIZE="10">
<INPUT TYPE="submit" NAME="sn_op" VALUE="play" SIZE="10">
<INPUT TYPE="submit" NAME="sn_op" VALUE="pause" SIZE="10">
<INPUT TYPE="submit" NAME="sn_op" VALUE="stop" SIZE="10">
<INPUT TYPE="submit" NAME="sn_op" VALUE="next" SIZE="10">
</FORM>
</P>
<HR>
<P CLASS="title">PLAYLIST COMMANDS</P>
<P>
<FORM METHOD="POST">
<INPUT TYPE="hidden" NAME="action" VALUE="playlist">
<INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
<INPUT TYPE="hidden" NAME="passwd" VALUE="<?php echo($passwd); ?>">
<INPUT TYPE="submit" NAME="pl_op" VALUE="view" SIZE="10">
<INPUT TYPE="submit" NAME="pl_op" VALUE="randomize" SIZE="10">
<INPUT TYPE="submit" NAME="pl_op" VALUE="sort by path" SIZE="10">
<INPUT TYPE="submit" NAME="pl_op" VALUE="sort by title" SIZE="10">
<INPUT TYPE="submit" NAME="pl_op" VALUE="remove deads" SIZE="10">
<INPUT TYPE="submit" NAME="pl_op" VALUE="clear" SIZE="10">
</FORM>
</P>
<HR>
<P CLASS="title">SOUNDS PARAMS</P>
<P>
<FORM METHOD="POST">
<INPUT TYPE="hidden" NAME="action" VALUE="sound">
<INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
<INPUT TYPE="hidden" NAME="passwd" VALUE="<?php echo($passwd); ?>">
volume &nbsp;&nbsp;: <INPUT TYPE="text" NAME="sn_volume" VALUE="<?php echo($sn_volume); ?>" SIZE="4"> %<BR>
panning &nbsp;: <INPUT TYPE="text" NAME="sn_pan" VALUE="<?php echo($sn_pan); ?>" SIZE="4"> %<BR>
position : <INPUT TYPE="text" NAME="sn_pos" VALUE="" SIZE="4"> %<BR><BR>
<INPUT TYPE="submit" NAME="sn_op" VALUE="modify" SIZE="10">
</FORM>
</P>
<HR>
<P CLASS="title">BROWSE PARAMS</P>
<P>
<FORM METHOD="POST">
<INPUT TYPE="hidden" NAME="action" VALUE="browse">
<INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
<INPUT TYPE="hidden" NAME="passwd" VALUE="<?php echo($passwd); ?>">
<INPUT TYPE="submit" NAME="bw_op" VALUE="get roots" SIZE="10">
</FORM><BR><BR>
<FORM METHOD="POST">
<INPUT TYPE="hidden" NAME="action" VALUE="browse">
<INPUT TYPE="hidden" NAME="host" VALUE="<?php echo($host); ?>">
<INPUT TYPE="hidden" NAME="passwd" VALUE="<?php echo($passwd); ?>">
path &nbsp;: <INPUT TYPE="text" NAME="bw_path" VALUE="<?php echo($bw_path); ?>" SIZE="30"><BR>
count : <INPUT TYPE="text" NAME="bw_maxcount" VALUE="<?php echo($bw_maxcount); ?>" SIZE="10"><BR>
<INPUT TYPE="submit" NAME="bw_op" VALUE="view" SIZE="10">
</FORM>
</P>
<?php

	}
	else
	{
		// wrong connection !
		echo("<P CLASS=\"error\">Cannot authenticate !</P>\n");
	}

	// closing connection with NGWinamp server
	$ngwc->disconnect();
}
else
{
	// cannot connect. in this case the connection properties are shown
	echo("<P CLASS=\"error\">Cannot connect !</P>\n");
?>
<HR>
<P CLASS="title">CONNECTION PARAMS</P>
<P>
<FORM METHOD="POST">
host &nbsp;&nbsp;: <INPUT TYPE="text" NAME="host" VALUE="<?php echo($host); ?>" SIZE="10"><BR>
passwd : <INPUT TYPE="password" NAME="passwd" VALUE="<?php echo($passwd); ?>" SIZE="10"><BR><BR>
<INPUT TYPE="submit" VALUE="connect" SIZE="10">
</FORM>
</P>
<?php
}
?>
</BODY>
</HTML>
