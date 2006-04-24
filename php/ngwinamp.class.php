<?php

/********************************************************/
/* NGWinamp Remote Control Client for PHP				*/
/********************************************************/
/* 														*/
/* Author  : Antony Ducommun							*/
/* Email   : nitro@webmails.com							*/
/* Date    : 10/11/2004									*/
/* Version : V1.1.0a									*/
/* Licence : Freeware :)								*/
/* 														*/
/* Made for NGWinamp V1.0.3 and higher.					*/
/* Best viewing with tabulation length to 4.			*/
/* 														*/
/* 														*/
/* 														*/
/********************************************************/
/* API DETAILS											*/
/********************************************************/
/* 														*/
/* 1) construct a new instance of NGWINAMPCLIENT class.	*/
/*     $ngwc = new NGWINAMPCLIENT;						*/
/*														*/
/* 2) connect to the remote NGWinamp server.			*/
/*     if ($ngwc->connect($host)) {						*/
/*          // connected !								*/
/*     } else {											*/
/*          // not connected !							*/
/*     }												*/
/*														*/
/* 3) authenticate the connection (always necessary).	*/
/*     $code = $ngwc->authenticate($password);			*/
/*     if ($code == NGWINAMP_AUTH_SUCCESS) {			*/
/*          // success !								*/
/*     }												*/
/*     if ($code == NGWINAMP_AUTH_FAILURE) {			*/
/*          // wrong passord or not authorized !		*/
/*     }												*/
/*     if ($code == NGWINAMP_AUTH_NOTDONE) {			*/
/*          // request sent without authorization !		*/
/*     }												*/
/*     if ($code == NGWINAMP_AUTH_TOOMANYCON) {			*/
/*          // too many connection at a given time !	*/
/*     }												*/
/*														*/
/* 4) use functions below to send & retrieve data with	*/
/*    NGWinamp server.									*/
/*														*/
/* Sound commands :										*/
/*     - $ngwc->sn_prev();								*/
/*     - $ngwc->sn_play();								*/
/*     - $ngwc->sn_pause();								*/
/*     - $ngwc->sn_stop();								*/
/*     - $ngwc->sn_next();								*/
/*     - $ngwc->sn_getvolume();							*/
/*     - $ngwc->sn_setvolume($volume);					*/
/*     - $ngwc->sn_getpan();							*/
/*     - $ngwc->sn_setpan($pan);						*/
/*     - $ngwc->sn_getpos();							*/
/*     - $ngwc->sn_setpos($pos);						*/
/*														*/
/* Playlist commands :									*/
/*     - $ngwc->pl_clear();								*/
/*     - $ngwc->pl_getnames();							*/
/*     - $ngwc->pl_getfiles();							*/
/*     - $ngwc->pl_setfiles($files);					*/
/*     - $ngwc->pl_addfiles($files);					*/
/*     - $ngwc->pl_delete($indexes);					*/
/*     - $ngwc->pl_swap($index1, $index2);				*/
/*     - $ngwc->pl_getpos();							*/
/*     - $ngwc->pl_setpos($pos);						*/
/*     - $ngwc->pl_getshuffle();						*/
/*     - $ngwc->pl_setshuffle($enable);					*/
/*     - $ngwc->pl_getrepeat();							*/
/*     - $ngwc->pl_setrepeat($enable);					*/
/*     - $ngwc->pl_randomize();							*/
/*     - $ngwc->pl_sortbypath();						*/
/*     - $ngwc->pl_sortbyname();						*/
/*     - $ngwc->pl_removedead();						*/
/*														*/
/* Browsing commands :									*/
/*     - $ngwc->bw_getroots();							*/
/*     - $ngwc->bw_getlist($path);						*/
/*														*/
/*														*/
/* 5) close the connection with NGWinamp server.		*/
/*     $ngwc->disconnect();								*/
/* 														*/
/* 														*/
/*														*/
/********************************************************/


// binary conversion utilities
function ngfrombyte($value)   { $a = unpack("Cvalue", $value); return $a['value']; }
function ngfromword($value)   { $a = unpack("vvalue", $value); return $a['value']; }
function ngfromdword($value)  { $a = unpack("Vvalue", $value); return $a['value']; }
function ngfromdouble($value) { $a = unpack("dvalue", $value); return $a['value']; }
function ngtobyte($value)     { return pack("C", $value); }
function ngtoword($value)     { return pack("v", $value); }
function ngtodword($value)    { return pack("V", $value); }
function ngtodouble($value)   { return pack("d", $value); }

// generic constants
define("NGWINAMP_NONE",					 0);
define("NGWINAMP_ALL",					-1);

// auth results
 // authorization successfull
define("NGWINAMP_AUTH_SUCCESS",			0x0);
 // authorization failure (wrong passord or not authorized)
define("NGWINAMP_AUTH_FAILURE",			0x1);
 // authorization not done (a command has been sent without authorization)
define("NGWINAMP_AUTH_NOTDONE",			0x2);
 // authorization failed (too many connection at a given time, or invalid source address)
define("NGWINAMP_AUTH_TOOMANYCON",		0x3);

// requests
define("NGWINAMP_REQ_NONE", 			0);
 // auth
define("NGWINAMP_REQ_AUTH", 			1);
 // sound
define("NGWINAMP_REQ_PREV", 			11);
define("NGWINAMP_REQ_PLAY", 			12);
define("NGWINAMP_REQ_PAUSE", 			13);
define("NGWINAMP_REQ_STOP", 			14);
define("NGWINAMP_REQ_NEXT", 			15);
define("NGWINAMP_REQ_GETVOLUME", 		21);
define("NGWINAMP_REQ_SETVOLUME", 		22);
define("NGWINAMP_REQ_GETPAN", 			23);
define("NGWINAMP_REQ_SETPAN", 			24);
define("NGWINAMP_REQ_GETPOS", 			25);
define("NGWINAMP_REQ_SETPOS", 			26);
 // playlist
define("NGWINAMP_REQ_PLCLEAR", 			100);
define("NGWINAMP_REQ_PLGETNAMES", 		101);
define("NGWINAMP_REQ_PLGETFILES", 		103);
define("NGWINAMP_REQ_PLSETFILES", 		104);
define("NGWINAMP_REQ_PLDELFILES", 		105);
define("NGWINAMP_REQ_PLADDFILES", 		106);
define("NGWINAMP_REQ_PLMOVEFILES", 		107);
define("NGWINAMP_REQ_PLGETPOS", 		111);
define("NGWINAMP_REQ_PLSETPOS",			112);
define("NGWINAMP_REQ_PLGETSHUFFLE",		121);
define("NGWINAMP_REQ_PLSETSHUFFLE",		122);
define("NGWINAMP_REQ_PLGETREPEAT", 		123);
define("NGWINAMP_REQ_PLSETREPEAT", 		124);
define("NGWINAMP_REQ_PLRANDOMIZE", 		130);
define("NGWINAMP_REQ_PLSORTBYNAME", 	131);
define("NGWINAMP_REQ_PLSORTBYPATH", 	132);
define("NGWINAMP_REQ_PLDELDEADFILES",	133);
 // browsing
define("NGWINAMP_REQ_BWGETROOTS", 		200);
define("NGWINAMP_REQ_BWGETLIST",		201);

// answers
define("NGWINAMP_ANS_NONE", 			0);
 // auth
define("NGWINAMP_ANS_AUTH", 			1);
 // sound
define("NGWINAMP_ANS_VOLUME", 			21);
define("NGWINAMP_ANS_PAN", 				23);
define("NGWINAMP_ANS_POS", 				25);
 // playlist
define("NGWINAMP_ANS_PLNAMES", 			101);
define("NGWINAMP_ANS_PLFILES", 			103);
define("NGWINAMP_ANS_PLBROWSE", 		107);
define("NGWINAMP_ANS_PLPOS", 			111);
define("NGWINAMP_ANS_PLSHUFFLE", 		121);
define("NGWINAMP_ANS_PLREPEAT", 		123);
 // browsing
define("NGWINAMP_ANS_BWROOTS", 			200);
define("NGWINAMP_ANS_BWLIST", 			201);




// Client interface
class NGWINAMPCLIENT {
	// NGWinamp server to connect
	var $curhost;
	var $curport;
	var $curtimeout;
	var $cursock;

	// NGWinamp answer storage
	var $curcode;
	var $curparam1;
	var $curparam2;
	var $curparam3;
	var $cursize;
	var $curdata;


	// Default constructor
	function __construct() {
		// Default host parameters
		$this->curhost = "localhost";
		$this->curport = 8443;
		$this->curtimeout = 30;
		$this->cursock = 0;

		// Clear answer storage
		$this->ngclear();
	}

	// Connect to a NGWinamp server
	function connect($host = "", $port = 0, $timeout = 0) {
		global $errno, $errstr;

		if ($host != "") {
			$this->curhost = $host;
		}
		if ($port > 0) {
			$this->curport = $port;
		}
		if ($timeout > 0) {
			$this->curtimeout = $timeout;
		}

		$this->cursock = fsockopen($this->curhost, $this->curport, &$errno, &$errstr, $this->curtimeout);
		if (!$this->cursock) {
			return FALSE;
		}
		socket_set_blocking($this->cursock, TRUE);
		socket_set_timeout($this->cursock, $this->curtimeout);

		$this->ngclear();
		return TRUE;
	}

	// Disconnect from NGWinamp server
	function disconnect() {
		if ($this->cursock) {
			fclose($this->cursock);
			$this->cursock = 0;
		}
		$this->ngclear();
	}


	// Authenticate the connection (do it before any other request)
	function authenticate($password = "") {
		// send request
		if (!$this->ngsend(NGWINAMP_REQ_AUTH, 0, 0, 0.0, strlen($password), $password)) {
			return NGWINAMP_AUTH_FAILURE;
		}

		// receive confirmation		
		do {
			if (!$this->ngrecv()) {
				return NGWINAMP_AUTH_FAILURE;
			}
		} while ($this->curcode != NGWINAMP_ANS_AUTH);

		// return authentification code (see top of this file)
		return $this->curparam1;
	}



	// Go to previous song
	function sn_prev() {
		return $this->ngsend(NGWINAMP_REQ_PREV);
	}
	// Play the current song
	function sn_play() {
		return $this->ngsend(NGWINAMP_REQ_PLAY);
	}
	// Pause/resume playing current song
	function sn_pause() {
		return $this->ngsend(NGWINAMP_REQ_PAUSE);
	}
	// Stop playing current song
	function sn_stop() {
		return $this->ngsend(NGWINAMP_REQ_STOP);
	}
	// Go to next song
	function sn_next() {
		return $this->ngsend(NGWINAMP_REQ_NEXT);
	}

	// Retrieve current volume (return from 0.0 to 100.0, in percent)
	function sn_getvolume() {
		if($this->ngsend(NGWINAMP_REQ_GETVOLUME)) {
			do {
				if (!$this->ngrecv()) {
					return 0.0;
				}
			} while ($this->curcode != NGWINAMP_ANS_VOLUME);
			return ($this->curparam3 * 100.0);
		}
		return 0.0;
	}
	// Set current volume (input from 0.0 to 100.0, in percent)
	function sn_setvolume($volume) {
		if ($volume < 0.0) {
			$volume = 0.0;
		}
		if ($volume > 100.0) {
			$volume = 100.0;
		}
		return $this->ngsend(NGWINAMP_REQ_SETVOLUME, 0, 0, ($volume / 100.0));
	}

	// Retrieve current panning (return from -100.0 to 100.0, in percent ; a negative value mean left and a postive one mean right)
	function sn_getpan() {
		if($this->ngsend(NGWINAMP_REQ_GETPAN)) {
			do {
				if (!$this->ngrecv()) {
					return 0.0;
				}
			} while ($this->curcode != NGWINAMP_ANS_PAN);
			return ($this->curparam3 * 100.0);
		}
		return 0.0;
	}
	// Set current panning (input from -100.0 to 100.0, in percent ; a negative value mean left and a postive one mean right)
	function sn_setpan($pan) {
		if ($pan < -100.0) {
			$pan = -100.0;
		}
		if ($pan > 100.0) {
			$pan = 100.0;
		}
		return $this->ngsend(NGWINAMP_REQ_SETPAN, 0, 0, ($pan / 100.0));
	}

	// Retrieve current song postion and length. (return an array with 'pos'      -> position in percent,
	// 																   'posms'    -> position in milliseconds,
	//																   'lengthms' -> length in milliseconds)
	function sn_getpos() {
		if($this->ngsend(NGWINAMP_REQ_GETPOS)) {
			do {
				if (!$this->ngrecv()) {
					return array('lengthms' => 0, 'posms' => 0, 'pos' => 0.0);
				}
			} while ($this->curcode != NGWINAMP_ANS_POS);
			return array('lengthms' => $this->curparam1, 'posms' => $this->curparam2, 'pos' => $this->curparam3 * 100.0);
		}
		return array('lengthms' => 0, 'posms' => 0, 'pos' => 0.0);
	}
	// Set current song postion. (input from 0.0 to 100.0, in percent)
	function sn_setpos($pos) {
		if ($pos < 0.0) {
			$pos = 0.0;
		}
		if ($pos > 100.0) {
			$pos = 100.0;
		}
		return $this->ngsend(NGWINAMP_REQ_SETPOS, 0, 0, ($pos / 100.0));
	}



	// Clear the current playlist
	function pl_clear() {
		return $this->ngsend(NGWINAMP_REQ_PLCLEAR);
	}

	// Retrieve one/all song's title in playlist (return an array of string)
	function pl_getnames($index = NGWINAMP_ALL) {
		if($this->ngsend(NGWINAMP_REQ_PLGETNAMES, $index)) {
			do {
				if (!$this->ngrecv()) {
					return array();
				}
			} while ($this->curcode != NGWINAMP_ANS_PLNAMES);

			$r = array();
			$offset = 0;
			for ($i = 0; $i < $this->curparam1; $i++) {
				$index = ngfromdword(substr($this->curdata, $offset, 4));
				$length = ngfromword(substr($this->curdata, $offset + 4, 2));
				$r[$i] = substr($this->curdata, $offset + 6, $length);
				$offset += 6 + $length;
			}
			return $r;
		}
		return array();
	}
	// Retrieve one/all song's filename in playlist (return an array of string)
	function pl_getfiles($index = NGWINAMP_ALL) {
		if ($this->ngsend(NGWINAMP_REQ_PLGETFILES, $index)) {
			do {
				if (!$this->ngrecv()) {
					return array();
				}
			} while ($this->curcode != NGWINAMP_ANS_PLFILES);

			$r = array();
			$offset = 0;
			for ($i = 0; $i < $this->curparam1; $i++) {
				$index = ngfromdword(substr($this->curdata, $offset, 4));
				$length = ngfromword(substr($this->curdata, $offset + 4, 2));
				$r[$i] = substr($this->curdata, $offset + 6, $length);
				$offset += 6 + $length;
			}
			return $r;
		}
		return array();
	}
	// Set playlist items (input is an array of filename)
	function pl_setfiles($files) {
		$length = 0;
		$count = 0;
		$data = "";
		for ($i = 0; $i < count($files); $i++) {
			if (strlen($files[$i]) > 0) {
				$length += 6 + strlen($files[$i]);
				$data .= ngtodword($i).ngtoword(strlen($files[$i])).$files[$i];
				$count++;
			}
		}
		if ($count > 0) {
			return $this->ngsend(NGWINAMP_REQ_PLSETFILES, $count, 0, 0.0, $length, $data);
		}
		return FALSE;
	}

	// Add playlist items (input is an array of filename)
	function pl_addfiles($files) {
		$length = 0;
		$count = 0;
		$data = "";
		for ($i = 0; $i < count($files); $i++) {
			if (strlen($files[$i]) > 0) {
				$length += 6 + strlen($files[$i]);
				$data .= ngtodword($i).ngtoword(strlen($files[$i])).$files[$i];
				$count++;
			}
		}
		if ($count > 0) {
			return $this->ngsend(NGWINAMP_REQ_PLADDFILES, $count, 0, 0.0, $length, $data);
		}
		return FALSE;
	}
	// Delete playlist items (input is an array of index ; between 0 and playlist's length)
	function pl_delete($indexes) {
		$data = "";
		for ($i = 0; $i < count($indexes); $i++) {
			$data .= ngtodword($indexes[$i]);
		}
		if (count($indexes) > 0) {
			return $this->ngsend(NGWINAMP_REQ_PLDELFILES, count($indexes), 0, 0, 4 * count($indexes), $data);
		}
		return FALSE;
	}
	// Swap two playlist item (input is the two indexes to swap)
	function pl_swap($index1, $index2) {
		$data = ngtodword($index1).ngtodword($index2);
		return $this->ngsend(NGWINAMP_REQ_PLMOVEFILES, 1, 0, 0, 2 * 4, $data);
	}

	// Retrieve current song index and playlist length (return an array with 'index'  -> current song index,
	//																		 'length' -> playlist length)
	function pl_getpos() {
		if ($this->ngsend(NGWINAMP_REQ_PLGETPOS)) {
			do {
				if (!$this->ngrecv()) {
					return array('index' => 0, 'length' => 0);
				}
			} while ($this->curcode != NGWINAMP_ANS_PLPOS);
			return array('index' => $this->curparam1, 'length' => $this->curparam2);
		}
		return array('index' => 0, 'length' => 0);
	}
	// Set current song (input is a zero based index in playlist)
	function pl_setpos($index) {
		return $this->ngsend(NGWINAMP_REQ_PLSETPOS, $index);
	}

	// Retrieve current shuffle mode (return is a boolean value : enabled or disabled)
	function pl_getshuffle() {
		if ($this->ngsend(NGWINAMP_REQ_PLGETSHUFFLE)) {
			do {
				if (!$this->ngrecv()) {
					return FALSE;
				}
			} while ($this->curcode != NGWINAMP_ANS_PLSHUFFLE);

			if ($this->curparam1 != NGWINAMP_NONE) {
				return TRUE;
			}
		}
		return FALSE;
	}
	// Change current shuffle mode (input is a boolean value : enable or disable)
	function pl_setshuffle($enable) {
		if ($enable) {
			return $this->ngsend(NGWINAMP_REQ_PLSETSHUFFLE, NGWINAMP_ALL);
		}
		return $this->ngsend(NGWINAMP_REQ_PLSETSHUFFLE, NGWINAMP_NONE);
	}

	// Retrieve current repeat mode (return is a boolean value : enabled or disabled)
	function pl_getrepeat() {
		if ($this->ngsend(NGWINAMP_REQ_PLGETREPEAT)) {
			do {
				if (!$this->ngrecv()) {
					return FALSE;
				}
			} while ($this->curcode != NGWINAMP_ANS_PLREPEAT);
			if ($this->curparam1 != NGWINAMP_NONE) {
				return TRUE;
			}
		}
		return FALSE;
	}
	// Change current repeat mode (input is a boolean value : enable or disable)
	function pl_setrepeat($enable) {
		if ($enable) {
			return $this->ngsend(NGWINAMP_REQ_PLSETREPEAT, NGWINAMP_ALL);
		}
		return $this->ngsend(NGWINAMP_REQ_PLSETREPEAT, NGWINAMP_NONE);
	}

	// Randomize playlist
	function pl_randomize() {
		return $this->ngsend(NGWINAMP_REQ_PLRANDOMIZE);
	}
	// Sort playlist by path and filename
	function pl_sortbypath() {
		return $this->ngsend(NGWINAMP_REQ_PLSORTBYPATH);
	}
	// Sort playlist by title
	function pl_sortbyname() {
		return $this->ngsend(NGWINAMP_REQ_PLSORTBYNAME);
	}

	// Remove dead songs from playlist
	function pl_removedead() {
		return $this->ngsend(NGWINAMP_REQ_PLDELDEADFILES);
	}



	// Retrieve authorized file roots (return an array of string)
	function bw_getroots() {
		if($this->ngsend(NGWINAMP_REQ_BWGETROOTS)) {
			do {
				if(!$this->ngrecv()) {
					return array();
				}
			} while ($this->curcode != NGWINAMP_ANS_BWROOTS);
			$offset = 0;
			$r = array();
			for($i = 0; $i < $this->curparam1; $i++) {
				$length = ngfromword(substr($this->curdata, $offset, 2));
				$r[$i] = substr($this->curdata, $offset + 2, $length);
				$offset += 2 + $length;
			}
			return $r;
		}
		return array();
	}
	// Retrieve content list from a path (return an array with 'directories' -> an array of string,
	//														   'files' -> an array of string)
	function bw_getlist($path) {
		if ($this->ngsend(NGWINAMP_REQ_BWGETLIST, 0, 0, 0.0, strlen($path), $path)) {
			do {
				if (!$this->ngrecv()) {
					return array('directories' => array(), 'files' => array());
				}
			} while ($this->curcode != NGWINAMP_ANS_BWLIST);

			$offset = 0;
			$r = array('directories' => array(), 'files' => array());
			for ($i = 0; $i < $this->curparam1; $i++) {
				$length = ngfromword(substr($this->curdata, $offset, 2));
				$r['directories'][$i] = substr($this->curdata, $offset + 2, $length);
				$offset += 2 + $length;
			}
			for ($i = 0; $i < $this->curparam2; $i++) {
				$length = ngfromword(substr($this->curdata, $offset, 2));
				$r['files'][$i] = substr($this->curdata, $offset + 2, $length);
				$offset += 2 + $length;
			}
			return $r;
		}
		return array('directories' => array(), 'files' => array());
	}







/************************************************/
/* Functions below are 'private' for internal	*/
/* calls only.									*/
/************************************************/
/* WARNING: don't call them directly !			*/
/************************************************/

	// Check if the connection is still alive
	function ngcheck($minsize = 0) {
		if ($this->cursock) {
			$state = socket_get_status($this->cursock);
			if ($state['unread_bytes'] < $minsize) {
				return FALSE;
			}
			if ($state['timed_out']) {
				return FALSE;
			}
			if ($state['eof']) {
				return FALSE;
			}
			return TRUE;
		}
		return FALSE;
	}

	// Receive answer from NGWinamp server (internal usage)
	function ngrecv() {
		$this->ngclear();
		if ($this->cursock && $this->ngcheck()) {
			// read header
			$offset = 0;
			$buffer = "";
			while ($offset < 32) {
				usleep(10);
				$buffertmp = fread($this->cursock, 32 - $offset);
				if (strlen($buffertmp) == 0) {
					return FALSE;
				}
				$offset += strlen($buffertmp);
				$buffer .= $buffertmp;
			}

			// parse header
			$code = ngfromdword(substr($buffer, 0, 4));
			$param1 = ngfromdword(substr($buffer, 4, 4));
			$param2 = ngfromdword(substr($buffer, 8, 4));
			$param3 = ngfromdouble(substr($buffer, 16, 8));
			$size = ngfromdword(substr($buffer, 24, 4));

			// read data buffer (if any)
			$offset = 0;
			$data = "";
			while ($offset < $size) {
				usleep(10);
				$datatmp = fread($this->cursock, $size - $offset);
				if (strlen($datatmp) == 0) {
					return FALSE;
				}
				$offset += strlen($datatmp);
				$data .= $datatmp;	
			}

			// save datas to answer storage
			$this->curcode = $code;
			$this->curparam1 = $param1;
			$this->curparam2 = $param2;
			$this->curparam3 = $param3;
			$this->cursize = $size;
			$this->curdata = $data;
			return TRUE;
		}
		return FALSE;
	}
	// Send request to NGWinamp server (internal usage)
	function ngsend($code, $param1 = 0, $param2 = 0, $param3 = 0.0, $size = 0, $data = "") {
		if ($this->cursock && $this->ngcheck()) {
			// send header + data
			$buffer = ngtodword($code).ngtodword($param1).ngtodword($param2).ngtodword(0).ngtodouble($param3).ngtodword($size).ngtodword(0).$data;
			$offset = 0;
			while ($offset < ($size + 32)) {
				$offset += fwrite($this->cursock, $buffer, $size - $offset + 32);
				usleep(10);
			}
			usleep(50);

			// flush connection
			fflush($this->cursock);
			return TRUE;
		}
		return FALSE;
	}
	// Clear answer storage (internal usage)
	function ngclear() {
		// set to zero
		$this->curcode = 0;
		$this->curparam1 = 0;
		$this->curparam2 = 0;
		$this->curparam3 = 0.0;
		$this->cursize = 0;
		$this->curdata = "";
	}
}

?>
