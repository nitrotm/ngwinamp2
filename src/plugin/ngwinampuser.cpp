// ngwinampuser.cpp
#include "plugin.h"


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


string NGWINAMPUSER::getusername() {
	NGLOCKER locker(this);

	return this->username;
}
string NGWINAMPUSER::getpassword() {
	NGLOCKER locker(this);

	return this->password;
}
dword NGWINAMPUSER::getaccess() {
	NGLOCKER locker(this);

	return this->access;
}
dword NGWINAMPUSER::getmaxcon() {
	NGLOCKER locker(this);

	return this->maxcon;
}
double NGWINAMPUSER::gettimeout() {
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
				if (!this->process(this->connections[i], prequest)) {
					this->connections[i]->shutdown();
				}
				delete prequest;
			}
		}
	}
	this->gc();
}

void NGWINAMPUSER::gc(void) {
	NGLOCKER locker(this);

	for (dword i = 0; i < this->connections.size(); i++) {
		if (this->connections[i]->isclosed()) {
			delete this->connections[i];
			this->connections.erase(this->connections.begin() + i);
			this->gc();
			break;
		}
	}
}

bool NGWINAMPUSER::process(NGWINAMPCON *pconnection, NETDATA *prequest) {
	NGLOCKER locker(this);
	NGBUFFER buffer;

	char tmp[512];
	sprintf(tmp, "NGWINAMPUSER::main() process (code=%u,param1=%08X,param2=%08X,flags=%08X,param3=%.02f,size=%08X,size2=%08X)", prequest->hdr.code, prequest->hdr.param1, prequest->hdr.param2, prequest->hdr.flags, prequest->hdr.param3, prequest->hdr.size, prequest->hdr.size2);
	DEBUGWRITE(tmp);

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
						string value = this->pserver->pl_getname(i);
						word   length = (word)value.length();

						buffer.append(&i, 4);
						buffer.append(&length, 2);
						buffer.append(value.c_str(), length);
					}
					count = total;
				} else {
					if (prequest->hdr.param1 < total) {
						string value = this->pserver->pl_getname(prequest->hdr.param1);
						word   length = (word)value.length();

						buffer.append(&prequest->hdr.param1, 4);
						buffer.append(&length, 2);
						buffer.append(value.c_str(), length);
						count = 1;
					}
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLNAMES, count, total, 0, 0.0, buffer))) {
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
						string value = this->pserver->pl_getfilename(i);
						word   length = (word)value.length();

						buffer.append(&i, 4);
						buffer.append(&length, 2);
						buffer.append(value.c_str(), length);
					}
					count = total;
				} else {
					if (prequest->hdr.param1 < total) {
						string value = this->pserver->pl_getfilename(prequest->hdr.param1);
						word   length = (word)value.length();

						buffer.append(&prequest->hdr.param1, 4);
						buffer.append(&length, 2);
						buffer.append(value.c_str(), length);
						count = 1;
					}
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_PLFILES, count, total, 0, 0.0, buffer))) {
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

		case NGWINAMP_REQ_BWGETROOTS:
			{
				word  length = 1;

				buffer.append(&length, 2);
				buffer.append("/", 1);
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_BWROOTS, 1, 0, 0, 0.0, buffer))) {
					return false;
				}
			}
			return true;
		case NGWINAMP_REQ_BWGETLIST:
			{
				vector<string>	dirs;
				vector<string>	files;
				string			path = prequest->buffer.tostring();

				strreplace(path, '\\', '/');
				if (path.length() > 0) {
					const FSNode *pnode = this->pserver->findshare(path);

					if (pnode != NULL) {
						dirs = pnode->listdirectories(this->username);
						files = pnode->listfiles(this->username);
					}
				}
				for (dword i = 0; i < dirs.size(); i++) {
					word   length = (word)dirs[i].length();

					buffer.append(&length, 2);
					buffer.append(dirs[i].c_str(), length);
				}
				for (dword i = 0; i < files.size(); i++) {
					word   length = (word)files[i].length();

					buffer.append(&length, 2);
					buffer.append(files[i].c_str(), length);
				}
				if (!pconnection->answer(new NETDATA(NGWINAMP_ANS_BWLIST, dirs.size(), files.size(), 0, 0.0, buffer))) {
					return false;
				}
			}
			return true;

		case NGWINAMP_REQ_GETSNAPSHOT:
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
		}
	}
	if (this->canwrite()) {
		switch (prequest->hdr.code) {
		case NGWINAMP_REQ_PREV:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_BACK)) {
				this->pserver->prev();
				return true;
			}
			break;
		case NGWINAMP_REQ_PLAY:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_PLAY)) {
				this->pserver->play();
				return true;
			}
			break;
		case NGWINAMP_REQ_PAUSE:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_PAUSE)) {
				this->pserver->pause();
				return true;
			}
			break;
		case NGWINAMP_REQ_STOP:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_STOP)) {
				this->pserver->stop();
				return true;
			}
			break;
		case NGWINAMP_REQ_NEXT:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_NEXT)) {
				this->pserver->next();
				return true;
			}
			break;

		case NGWINAMP_REQ_SETVOLUME:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_VOLUME)) {
				this->pserver->sn_setvolume(prequest->hdr.param3);
				return true;
			}
			break;
		case NGWINAMP_REQ_SETPAN:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_PAN)) {
				this->pserver->sn_setpan(prequest->hdr.param3);
				return true;
			}
			break;
		case NGWINAMP_REQ_SETPOS:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_SN_POS)) {
				this->pserver->sn_setpos(prequest->hdr.param3);
				return true;
			}
			break;

		case NGWINAMP_REQ_PLADDFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_ADD)) {
				dword offset = 0;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					vector<string>	paths;
					string			filename;
					dword			index;
					word			length;

					if (prequest->buffer.read(&index, offset + 0, 4) != 4 ||
						prequest->buffer.read(&length, offset + 4, 2) != 2) {
						return false;
					}
					filename = prequest->buffer.tostring(offset + 6, length);
					if (filename.length() != length) {
						return false;
					}
					offset += 6 + length;

					paths = this->pserver->getfilepaths(this->username, filename);
					this->pserver->pl_addfile(paths);
				}
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETPOS:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_setpos(prequest->hdr.param1);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETSHUFFLE:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_setshuffle((prequest->hdr.param1 != NGWINAMP_NONE) ? true : false);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLSETREPEAT:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_CTRL)) {
				this->pserver->pl_setrepeat((prequest->hdr.param1 != NGWINAMP_NONE) ? true : false);
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
				dword offset = 0;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					vector<string>	paths;
					string			filename;
					dword			index;
					word			length;

					if (prequest->buffer.read(&index, offset + 0, 4) != 4 ||
						prequest->buffer.read(&length, offset + 4, 2) != 2) {
						return false;
					}
					filename = prequest->buffer.tostring(offset + 6, length);
					if (filename.length() != length) {
						return false;
					}
					offset += 6 + length;

					paths = this->pserver->getfilepaths(this->username, filename);
					this->pserver->pl_setfile(index, paths);
				}
				return true;
			}
			break;
		case NGWINAMP_REQ_PLDELFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_DEL)) {
				vector<dword>	items;
				dword			offset = 0;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					dword index;

					if (prequest->buffer.read(&index, offset + 0, 4) != 4) {
						return false;
					}
					offset += 4;
					items.push_back(index);
				}
				sort(items.begin(), items.end());
				reverse(items.begin(), items.end());
				this->pserver->pl_delfile(items);
				return true;
			}
			break;
		case NGWINAMP_REQ_PLMOVEFILES:
			if (this->hasaccess(NGWINAMPUSER_ACCESS_PL_SET)) {
				dword offset = 0;

				for (dword i = 0; i < prequest->hdr.param1; i++) {
					dword  index1, index2;

					if (prequest->buffer.read(&index1, offset + 0, 4) != 4 ||
						prequest->buffer.read(&index2, offset + 4, 2) != 4) {
						return false;
					}
					offset += 8;
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
//		switch (prequest->hdr.code) {
//		}
		return true;
	}
	DEBUGWRITE("Error processing request for " + this->username + " !");
	return false;
}


dword NGWINAMPUSER::authenticate(const string &password, const SOCKADDR_IN &address) {
	NGLOCKER locker(this);

	char tmp[512];
	sprintf(tmp, "NGWINAMPUSER::authenticate() login (username=%s,password=%s,ip=%u.%u.%u.%u)", this->username.c_str(), password.c_str(),
		address.sin_addr.S_un.S_un_b.s_b1, address.sin_addr.S_un.S_un_b.s_b2, address.sin_addr.S_un.S_un_b.s_b3, address.sin_addr.S_un.S_un_b.s_b4);
	DEBUGWRITE(tmp);

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
