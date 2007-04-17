// ngwinampuser.cpp
#include "../global.h"
#include "../util.h"
#include "../config.h"
#include "../fs.h"
#include "../net.h"
#include "../netaddr.h"
#include "../netdata.h"
#include "../netsnapshot.h"
#include "../netauth.h"
#include "plugin.h"
#include "ngwinamp.h"
#include "ngwinampserver.h"
#include "ngwinampcon.h"
#include "ngwinampuser.h"


NGWINAMPUSER::NGWINAMPUSER(NGWINAMPSERVER *pserver, const string &username, const string &password) : NGLOCK(), pserver(pserver), username(username), password(password), access(0), maxcon(0), timeout(0.0), byte_in(0), byte_out(0) {
}
NGWINAMPUSER::~NGWINAMPUSER() {
	this->close();
}


void NGWINAMPUSER::close(void) {
	NGLOCKER locker(this);

	for (dword i = 0; i < this->connections.size(); i++) {
		delete this->connections[i];
	}
	this->connections.clear();
}


dword NGWINAMPUSER::parsepolicies(CFGNode &access) {
	dword			ret = NGWINAMPUSER_ACCESS_NONE;
	vector<string>	winamp;
	vector<string>	playlist;

	if (access.getchild("read").getbool()) {
		ret |= NGWINAMPUSER_ACCESS_READ;
	}
	if (access.getchild("admin").getbool()) {
		ret |= NGWINAMPUSER_ACCESS_ADMIN;
	}
	winamp = strsplit(access.getchild("winamp").getstr(), " ", 0);
	for (dword i = 0; i < winamp.size(); i++) {
		if (winamp[i].compare("back") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_BACK;
		} else if (winamp[i].compare("play") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_PLAY;
		} else if (winamp[i].compare("pause") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_PAUSE;
		} else if (winamp[i].compare("stop") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_STOP;
		} else if (winamp[i].compare("next") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_NEXT;
		} else if (winamp[i].compare("volume") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_VOLUME;
		} else if (winamp[i].compare("pan") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_PAN;
		} else if (winamp[i].compare("pos") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_SN_POS;
		}
	}

	playlist = strsplit(access.getchild("playlist").getstr(), " ", 0);
	for (dword i = 0; i < playlist.size(); i++) {
		if (playlist[i].compare("add") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_PL_ADD;
		} else if (playlist[i].compare("set") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_PL_SET;
		} else if (playlist[i].compare("del") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_PL_DEL;
		} else if (playlist[i].compare("ctrl") == 0) {
			ret |= NGWINAMPUSER_ACCESS_WRITE;
			ret |= NGWINAMPUSER_ACCESS_PL_CTRL;
		}
	}
	return ret;
}

void NGWINAMPUSER::setpolicies(dword access, dword maxcon, double timeout) {
	NGLOCKER locker(this);

	this->access = access;
	this->maxcon = maxcon;
	this->timeout = timeout;
}

void NGWINAMPUSER::deny(const NETADDR &addr) {
	NGLOCKER locker(this);

	this->denied.push_back(addr);
}

void NGWINAMPUSER::allow(const NETADDR &addr) {
	NGLOCKER locker(this);

	this->allowed.push_back(addr);
}

string NGWINAMPUSER::getusername(void) {
	NGLOCKER locker(this);

	return this->username;
}
string NGWINAMPUSER::getpassword(void) {
	NGLOCKER locker(this);

	return this->password;
}
dword NGWINAMPUSER::getaccess(void) {
	NGLOCKER locker(this);

	return this->access;
}
dword NGWINAMPUSER::getcurcon(void) {
	NGLOCKER locker(this);

	return this->connections.size();
}
dword NGWINAMPUSER::getmaxcon(void) {
	NGLOCKER locker(this);

	return this->maxcon;
}
double NGWINAMPUSER::gettimeout(void) {
	NGLOCKER locker(this);

	return this->timeout;
}
bool NGWINAMPUSER::hasaccess(dword what) {
	NGLOCKER locker(this);

	return ((this->access & what) == what);
}
bool NGWINAMPUSER::canread(void) {
	NGLOCKER locker(this);

	return this->hasaccess(NGWINAMPUSER_ACCESS_READ);
}
bool NGWINAMPUSER::canwrite(void) {
	NGLOCKER locker(this);

	return this->hasaccess(NGWINAMPUSER_ACCESS_WRITE);
}
bool NGWINAMPUSER::canadmin(void) {
	NGLOCKER locker(this);

	return this->hasaccess(NGWINAMPUSER_ACCESS_ADMIN);
}

dword NGWINAMPUSER::getbytein(void) {
	NGLOCKER locker(this);

	return this->byte_in;
}
dword NGWINAMPUSER::getbyteout(void) {
	NGLOCKER locker(this);

	return this->byte_out;
}


bool NGWINAMPUSER::exists(NGWINAMPCON *pconnection) {
	NGLOCKER locker(this);

	for (dword i = 0; i < this->connections.size(); i++) {
		if (this->connections[i] == pconnection) {
			return true;
		}
	}
	return false;
}

void NGWINAMPUSER::add(NGWINAMPCON *pconnection) {
	NGLOCKER locker(this);

	if (!this->exists(pconnection)) {
		this->connections.push_back(pconnection);
	}
}


void NGWINAMPUSER::main(void) {
	NGLOCKER locker(this);

	for (dword i = 0; i < this->connections.size(); i++) {
		if (this->connections[i]->recvmsg() && this->connections[i]->sendmsg()) {
			NETDATA *prequest = this->connections[i]->request();

			if (prequest != NULL) {
				prequest->uncompress();
				if (!this->process(this->connections[i], prequest)) {
					this->connections[i]->shutdown();
				}
				delete prequest;
			} else {
				if (this->connections[i]->checksnapshot()) {
					if (!this->sendsnapshot(this->connections[i])){
						this->connections[i]->shutdown();
					}
				}
			}
		}
	}
	this->gc();
}

void NGWINAMPUSER::gc(void) {
	NGLOCKER locker(this);

	for (long i = this->connections.size() - 1; i >= 0; i--) {
		if (this->connections[i]->isclosed()) {
			this->byte_in += this->connections[i]->getbytein();
			this->byte_out += this->connections[i]->getbyteout();

#ifdef _DEBUG
			char tmp[512];
			sprintf(tmp, "NGWINAMPUSER::gc() user %s connection closed (in: %u / %u [b], out: %u / %u [b])", this->username.c_str(), this->connections[i]->getbytein(), this->byte_in, this->connections[i]->getbyteout(), this->byte_out);
			DEBUGWRITE(tmp);
#endif

			delete this->connections[i];
			this->connections.erase(this->connections.begin() + i);
		}
	}
}

bool NGWINAMPUSER::process(NGWINAMPCON *pconnection, NETDATA *prequest) {
	NGLOCKER locker(this);
	NGREADER reader(prequest->buffer);
	NGWRITER writer;

#ifdef _DEBUG
	char tmp[512];
	sprintf(tmp, "NGWINAMPUSER::main() process (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", prequest->hdr.code, prequest->hdr.param1, prequest->hdr.param2, prequest->hdr.flags, prequest->hdr.param3, prequest->hdr.size, prequest->hdr.size2);
	DEBUGWRITE(tmp);
#endif

	if (this->canread()) {
		switch (prequest->hdr.code) {
		case NGWINAMP_REQ_GETVOLUME:
			NGWINAMP_SNAPSHOT_SN_SETVOLUME(pconnection, this->pserver->sn_getvolume());
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_VOLUME, 0, 0, 0, this->pserver->sn_getvolume()))) {
				return false;
			}
			return true;
		case NGWINAMP_REQ_GETPAN:
			NGWINAMP_SNAPSHOT_SN_SETPAN(pconnection, this->pserver->sn_getpan());
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PAN, 0, 0, 0, this->pserver->sn_getpan()))) {
				return false;
			}
			return true;
		case NGWINAMP_REQ_GETPOS:
			NGWINAMP_SNAPSHOT_SN_SETPOSMS(pconnection, this->pserver->sn_getposms());
			NGWINAMP_SNAPSHOT_SN_SETLENGTH(pconnection, this->pserver->sn_getlength());
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_POS, this->pserver->sn_getlength(), this->pserver->sn_getposms(), 0, this->pserver->sn_getpos()))) {
				return false;
			}
			return true;

		case NGWINAMP_REQ_PLGETNAMES:
			{
				dword count = 0;
				dword total = this->pserver->pl_getlength();

				if (prequest->hdr.param1 == NGWINAMP_ALL) {
					for (dword i = 0; i < total; i++) {
						writer.writeDword(i);
						writer.writeWordString(this->pserver->pl_getname(i));
					}
					count = total;
				} else {
					if (prequest->hdr.param1 < total) {
						writer.writeDword(prequest->hdr.param1);
						writer.writeWordString(this->pserver->pl_getname(prequest->hdr.param1));
						count = 1;
					}
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLNAMES, count, total, 0, 0.0, writer))) {
					return false;
				}
			}
			return true;
		case NGWINAMP_REQ_PLGETFILES:
			{
				dword count = 0;
				dword total = this->pserver->pl_getlength();

				if (prequest->hdr.param1 == NGWINAMP_ALL) {
					for (dword i = 0; i < total; i++) {
						writer.writeDword(i);
						writer.writeWordString(this->pserver->pl_getfilename(i));
					}
					count = total;
				} else {
					if (prequest->hdr.param1 < total) {
						writer.writeDword(prequest->hdr.param1);
						writer.writeWordString(this->pserver->pl_getfilename(prequest->hdr.param1));
						count = 1;
					}
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLFILES, count, total, 0, 0.0, writer))) {
					return false;
				}
			}
			return true;
		case NGWINAMP_REQ_PLGETPOS:
			NGWINAMP_SNAPSHOT_PL_SETPOS(pconnection, this->pserver->pl_getpos());
			NGWINAMP_SNAPSHOT_PL_SETLENGTH(pconnection, this->pserver->pl_getlength());
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLPOS, this->pserver->pl_getpos(), this->pserver->pl_getlength(), 0, 0.0))) {
				return false;
			}
			return true;
		case NGWINAMP_REQ_PLGETSHUFFLE:
			NGWINAMP_SNAPSHOT_PL_SETSHUFFLE(pconnection, this->pserver->pl_getshuffle());
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLSHUFFLE, this->pserver->pl_getshuffle() ? NGWINAMP_ALL : NGWINAMP_NONE, 0, 0, 0.0))) {
				return false;
			}
			return true;
		case NGWINAMP_REQ_PLGETREPEAT:
			NGWINAMP_SNAPSHOT_PL_SETREPEAT(pconnection, this->pserver->pl_getrepeat());
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLREPEAT, this->pserver->pl_getrepeat() ? NGWINAMP_ALL : NGWINAMP_NONE, 0, 0, 0.0))) {
				return false;
			}
			return true;

		case NGWINAMP_REQ_BWGETROOTS: // note: obsolete
			{
				writer.writeWordString("/");
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_BWROOTS, 1, 0, 0, 0.0, writer))) {
					return false;
				}
			}
			return true;
		case NGWINAMP_REQ_BWGETLIST: // note: obsolete
			{
				vector<string>	dirs;
				vector<string>	files;
				string			path = prequest->buffer.tostring();

				path = strreplace(path, '\\', '/');
				if (path.length() > 0) {
					const FSNode *pnode = this->pserver->findshare(path);

					if (pnode != NULL) {
						dirs = pnode->listdirectories(this->username);
						files = pnode->listfiles(this->username);
					}
				}
				for (dword i = 0; i < dirs.size(); i++) {
					writer.writeWordString(dirs[i]);
				}
				for (dword i = 0; i < files.size(); i++) {
					writer.writeWordString(files[i]);
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_BWLIST, dirs.size(), files.size(), 0, 0.0, writer))) {
					return false;
				}
			}
			return true;

		case NGWINAMP_REQ_GETSNAPSHOT: // note: obsolete
			NGWINAMP_SNAPSHOT_SN_SETVOLUME(pconnection, this->pserver->sn_getvolume());
			if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_VOLUME)) {
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_VOLUME, 0, 0, 0, this->pserver->sn_getvolume()))) {
					return false;
				}
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_VOLUME);
			}

			NGWINAMP_SNAPSHOT_SN_SETPAN(pconnection, this->pserver->sn_getpan());
			if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_PAN)) {
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PAN, 0, 0, 0, this->pserver->sn_getpan()))) {
					return false;
				}
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_PAN);
			}

			NGWINAMP_SNAPSHOT_SN_SETPOSMS(pconnection, this->pserver->sn_getposms());
			NGWINAMP_SNAPSHOT_SN_SETLENGTH(pconnection, this->pserver->sn_getlength());
			if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_POSMS | NGWINAMP_SNAPSHOT_SN_LENGTH)) {
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_POS, this->pserver->sn_getlength(), this->pserver->sn_getposms(), 0, this->pserver->sn_getpos()))) {
					return false;
				}
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_POSMS | NGWINAMP_SNAPSHOT_SN_LENGTH);
			}

			NGWINAMP_SNAPSHOT_PL_SETPOS(pconnection, this->pserver->pl_getpos());
			NGWINAMP_SNAPSHOT_PL_SETLENGTH(pconnection, this->pserver->pl_getlength());
			if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_POS | NGWINAMP_SNAPSHOT_PL_LENGTH)) {
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLPOS, this->pserver->pl_getpos(), this->pserver->pl_getlength(), 0, 0.0))) {
					return false;
				}
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_POS | NGWINAMP_SNAPSHOT_PL_LENGTH);
			}

			NGWINAMP_SNAPSHOT_PL_SETSHUFFLE(pconnection, this->pserver->pl_getshuffle());
			if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_SHUFFLE)) {
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLSHUFFLE, this->pserver->pl_getshuffle() ? NGWINAMP_ALL : NGWINAMP_NONE, 0, 0, 0.0))) {
					return false;
				}
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_SHUFFLE);
			}

			NGWINAMP_SNAPSHOT_PL_SETREPEAT(pconnection, this->pserver->pl_getrepeat());
			if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_REPEAT)) {
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLREPEAT, this->pserver->pl_getrepeat() ? NGWINAMP_ALL : NGWINAMP_NONE, 0, 0, 0.0))) {
					return false;
				}
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_REPEAT);
			}
			return true;

		case NGWINAMP_REQ_BWGETDIRECTORIES:
			{
				vector<FSNode*>	dirs;
				const FSNode	*pnode;
				string			path;

				path = reader.readWordString();
				if (reader.isError()) {
					return false;
				}
				if (path.length() == 0) {
					path += "/";
				} else if (path[path.length() - 1] != '/') {
					path += "/";
				}
				pnode = this->pserver->findshare(path);
				if (pnode != NULL) {
					vector<string> names = pnode->listdirectories(this->username);

					for (dword i = 0; i < names.size(); i++) {
						dirs.push_back(pnode->get(names[i]));
					}
				}
				writer.writeWordString(path);
				for (dword i = 0; i < dirs.size(); i++) {
					writer.writeDword(dirs[i]->gettype());
					writer.writeDword(dirs[i]->getsubdirectorycount(this->username));
					writer.writeWordString(dirs[i]->getname());
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_BWDIRECTORIES, dirs.size(), 0, 0, 0.0, writer))) {
					return false;
				}
			}
			return true;

		case NGWINAMP_REQ_BWGETFILES:
			{
				vector<FSNode*>	files;
				const FSNode	*pnode;
				string			path;

				path = reader.readWordString();
				if (reader.isError()) {
					return false;
				}
				if (path.length() == 0) {
					path += "/";
				} else if (path[path.length() - 1] != '/') {
					path += "/";
				}
				pnode = this->pserver->findshare(path);
				if (pnode != NULL) {
					vector<string> names = pnode->listfiles(this->username);

					for (dword i = 0; i < names.size(); i++) {
						files.push_back(pnode->get(names[i]));
					}
				}
				writer.writeWordString(path);
				for (dword i = 0; i < files.size(); i++) {
					writer.writeDword(files[i]->gettype());
					writer.writeDword(files[i]->getsize());
					writer.writeWordString(files[i]->getname());
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_BWFILES, files.size(), 0, 0, 0.0, writer))) {
					return false;
				}
			}
			return true;

		case NGWINAMP_REQ_GETSNAPSHOT_EX:
			pconnection->setsnapshot(prequest->hdr.param3);
			return this->sendsnapshot(pconnection);
		}
	}
	if (this->canwrite()) {
		switch (prequest->hdr.code) {
		case NGWINAMP_REQ_PREV:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_BACK)) {
				this->pserver->sn_prev();
				return true;
			}
			break;
		case NGWINAMP_REQ_PLAY:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_PLAY)) {
				this->pserver->sn_play();
				return true;
			}
			break;
		case NGWINAMP_REQ_PAUSE:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_PAUSE)) {
				this->pserver->sn_pause();
				return true;
			}
			break;
		case NGWINAMP_REQ_STOP:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_STOP)) {
				this->pserver->sn_stop();
				return true;
			}
			break;
		case NGWINAMP_REQ_NEXT:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_NEXT)) {
				this->pserver->sn_next();
				return true;
			}
			break;

		case NGWINAMP_REQ_SETVOLUME:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_VOLUME)) {
				this->pserver->sn_setvolume(prequest->hdr.param3);
				pconnection->snapshot.sn_volume = this->pserver->sn_getvolume();
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_VOLUME);
				return true;
			}
			break;
		case NGWINAMP_REQ_SETPAN:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_PAN)) {
				this->pserver->sn_setpan(prequest->hdr.param3);
				pconnection->snapshot.sn_pan = this->pserver->sn_getpan();
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_PAN);
				return true;
			}
			break;
		case NGWINAMP_REQ_SETPOS:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_POS)) {
				this->pserver->sn_setpos(prequest->hdr.param3);
				pconnection->snapshot.sn_posms = this->pserver->sn_getposms();
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_POSMS);
				return true;
			}
			break;

		case NGWINAMP_REQ_PLADDFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_ADD)) {
				for (dword i = 0; i < prequest->hdr.param1; i++) {
					vector<string>	paths;
					dword			index = reader.readDword();
					string			filename = reader.readWordString();

					if (reader.isError()) {
						return false;
					}
					paths = this->pserver->getfilepaths(this->username, filename);
					this->pserver->pl_insfile(index, paths);
				}
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETPOS:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_setpos(prequest->hdr.param1);
				pconnection->snapshot.pl_pos = this->pserver->pl_getpos();
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_POS);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETSHUFFLE:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_setshuffle((prequest->hdr.param1 != NGWINAMP_NONE) ? true : false);
				pconnection->snapshot.pl_shuffle = this->pserver->pl_getshuffle();
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_SHUFFLE);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETREPEAT:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_setrepeat((prequest->hdr.param1 != NGWINAMP_NONE) ? true : false);
				pconnection->snapshot.pl_repeat = this->pserver->pl_getrepeat();
				NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_REPEAT);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLCLEAR:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_DEL)) {
				this->pserver->pl_clear();
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_SET)) {
				for (dword i = 0; i < prequest->hdr.param1; i++) {
					vector<string>	paths;
					dword			index = reader.readDword();
					string			filename = reader.readWordString();

					if (reader.isError()) {
						return false;
					}
					paths = this->pserver->getfilepaths(this->username, filename);
					this->pserver->pl_setfile(index, paths);
				}
				return true;
			}
			break;
		case NGWINAMP_REQ_PLDELFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_DEL)) {
				vector<dword> items;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					items.push_back(reader.readDword());
					if (reader.isError()) {
						return false;
					}
				}
				sort(items.begin(), items.end());
				reverse(items.begin(), items.end());
				this->pserver->pl_delfile(items);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLMOVEFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_SET)) {
				for (dword i = 0; i < prequest->hdr.param1; i++) {
					dword index1 = reader.readDword();
					dword index2 = reader.readDword();

					if (reader.isError()) {
						return false;
					}
					this->pserver->pl_swapfile(index1, index2);
				}
				return true;
			}
			break;
		case NGWINAMP_REQ_PLRANDOMIZE:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_randomize();
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSORTBYNAME:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_sortbyname();
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSORTBYPATH:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_sortbypath();
				return true;
			}
			break;
		case NGWINAMP_REQ_PLDELDEADFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_removedeadfiles();
				return true;
			}
			break;
		}
	}
	if (this->canadmin()) {
		switch (prequest->hdr.code) {
		case NGWINAMP_REQA_GETSHARES:
			// note: TODO
			return true;

		case NGWINAMP_REQA_SETSHARE:
			// note: TODO
			return true;

		case NGWINAMP_REQA_ADDSHARE:
			// note: TODO
			return true;

		case NGWINAMP_REQA_DELSHARE:
			// note: TODO
			return true;

		case NGWINAMP_REQA_GETUSERS:
			// note: TODO
			return true;

		case NGWINAMP_REQA_SETUSER:
			// note: TODO
			return true;

		case NGWINAMP_REQA_ADDUSER:
			// note: TODO
			return true;

		case NGWINAMP_REQA_DELUSER:
			// note: TODO
			return true;

		case NGWINAMP_REQA_GETCLIENTS:
			// note: TODO
			return true;

		case NGWINAMP_REQA_KILLCLIENT:
			// note: TODO
			return true;

		case NGWINAMP_REQA_RESTART:
			// note: TODO
			return true;
		}
		return true;
	}
	DEBUGWRITE("Error processing request for " + this->username + " !");
	return false;
}

bool NGWINAMPUSER::sendsnapshot(NGWINAMPCON *pconnection) {
	NGLOCKER locker(this);
	NGWRITER writer;

	if (this->canread()) {
		dword count = 0;

		NGWINAMP_SNAPSHOT_SN_SETVOLUME(pconnection, this->pserver->sn_getvolume());
		if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_VOLUME)) {
			writer.writeDword(NGWINAMP_SNAPSHOT_SN_VOLUME);
			writer.writeDword(sizeof(double));
			writer.writeDouble(this->pserver->sn_getvolume());
			NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_VOLUME);
			count++;
		}

		NGWINAMP_SNAPSHOT_SN_SETPAN(pconnection, this->pserver->sn_getpan());
		if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_PAN)) {
			writer.writeDword(NGWINAMP_SNAPSHOT_SN_PAN);
			writer.writeDword(sizeof(double));
			writer.writeDouble(this->pserver->sn_getpan());
			NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_PAN);
			count++;
		}

		NGWINAMP_SNAPSHOT_SN_SETPOSMS(pconnection, this->pserver->sn_getposms());
		NGWINAMP_SNAPSHOT_SN_SETLENGTH(pconnection, this->pserver->sn_getlength());
		if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_POSMS | NGWINAMP_SNAPSHOT_SN_LENGTH)) {
			writer.writeDword(NGWINAMP_SNAPSHOT_SN_POSMS | NGWINAMP_SNAPSHOT_SN_LENGTH);
			writer.writeDword(sizeof(dword) * 2);
			writer.writeDword(this->pserver->sn_getposms());
			writer.writeDword(this->pserver->sn_getlength());
			NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_SN_POSMS | NGWINAMP_SNAPSHOT_SN_LENGTH);
			count++;
		}

		NGWINAMP_SNAPSHOT_PL_SETPOS(pconnection, this->pserver->pl_getpos());
		NGWINAMP_SNAPSHOT_PL_SETLENGTH(pconnection, this->pserver->pl_getlength());
		if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_POS | NGWINAMP_SNAPSHOT_PL_LENGTH)) {
			writer.writeDword(NGWINAMP_SNAPSHOT_PL_POS | NGWINAMP_SNAPSHOT_PL_LENGTH);
			writer.writeDword(sizeof(dword) * 2);
			writer.writeDword(this->pserver->pl_getpos());
			writer.writeDword(this->pserver->pl_getlength());
			NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_POS | NGWINAMP_SNAPSHOT_PL_LENGTH);
			count++;
		}

		NGWINAMP_SNAPSHOT_PL_SETSHUFFLE(pconnection, this->pserver->pl_getshuffle());
		if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_SHUFFLE)) {
			writer.writeDword(NGWINAMP_SNAPSHOT_PL_SHUFFLE);
			writer.writeDword(sizeof(byte));
			writer.writeBool(this->pserver->pl_getshuffle());
			NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_SHUFFLE);
			count++;
		}

		NGWINAMP_SNAPSHOT_PL_SETREPEAT(pconnection, this->pserver->pl_getrepeat());
		if (NGWINAMP_SNAPSHOT_HASFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_REPEAT)) {
			writer.writeDword(NGWINAMP_SNAPSHOT_PL_REPEAT);
			writer.writeDword(sizeof(byte));
			writer.writeBool(this->pserver->pl_getrepeat());
			NGWINAMP_SNAPSHOT_RESETFLAG(pconnection, NGWINAMP_SNAPSHOT_PL_REPEAT);
			count++;
		}

		if (count > 0) {
			if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_SNAPSHOT_EX, count, 0, 0, 0.0, writer))) {
				return false;
			}
		}
		return true;
	}
	return false;
}

dword NGWINAMPUSER::authenticate(const string &password, const SOCKADDR_IN &address) {
	NGLOCKER locker(this);

#ifdef _DEBUG
	char tmp[512];
	sprintf(tmp, "NGWINAMPUSER::authenticate() login (username=%s,password=%s,ip=%u.%u.%u.%u)", this->username.c_str(), password.c_str(),
		address.sin_addr.S_un.S_un_b.s_b1, address.sin_addr.S_un.S_un_b.s_b2, address.sin_addr.S_un.S_un_b.s_b3, address.sin_addr.S_un.S_un_b.s_b4);
	DEBUGWRITE(tmp);
#endif

	if (this->connections.size() >= this->maxcon) {
		return NGWINAMP_AUTH_TOOMANYCON;
	}
	if (this->password.compare(password) != 0) {
		return NGWINAMP_AUTH_FAILURE;
	}
	for (dword i = 0; i < this->allowed.size(); i++) {
		if ((address.sin_addr.S_un.S_un_b.s_b1 & this->allowed[i].mask[0]) == (this->allowed[i].ip[0] & this->allowed[i].mask[0])) {
			if ((address.sin_addr.S_un.S_un_b.s_b2 & this->allowed[i].mask[1]) == (this->allowed[i].ip[1] & this->allowed[i].mask[1])) {
				if ((address.sin_addr.S_un.S_un_b.s_b3 & this->allowed[i].mask[2]) == (this->allowed[i].ip[2] & this->allowed[i].mask[2])) {
					if ((address.sin_addr.S_un.S_un_b.s_b4 & this->allowed[i].mask[3]) == (this->allowed[i].ip[3] & this->allowed[i].mask[3])) {
						return NGWINAMP_AUTH_SUCCESS;
					}
				}
			}
		}
	}
	for (dword i = 0; i < this->denied.size(); i++) {
		if ((address.sin_addr.S_un.S_un_b.s_b1 & this->denied[i].mask[0]) == (this->denied[i].ip[0] & this->denied[i].mask[0])) {
			if ((address.sin_addr.S_un.S_un_b.s_b2 & this->denied[i].mask[1]) == (this->denied[i].ip[1] & this->denied[i].mask[1])) {
				if ((address.sin_addr.S_un.S_un_b.s_b3 & this->denied[i].mask[2]) == (this->denied[i].ip[2] & this->denied[i].mask[2])) {
					if ((address.sin_addr.S_un.S_un_b.s_b4 & this->denied[i].mask[3]) == (this->denied[i].ip[3] & this->denied[i].mask[3])) {
						return NGWINAMP_AUTH_FAILURE;
					}
				}
			}
		}
	}
	if (this->allowed.size() == 0) {
		return NGWINAMP_AUTH_SUCCESS;
	}
	return NGWINAMP_AUTH_FAILURE;
}
