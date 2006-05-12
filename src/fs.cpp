// fs.cpp
#include "global.h"
#include "fs.h"


FSNode::FSNode(FSNode *parent, const string &name) : parent(parent), type(FS_TYPE_NONE), size(0), name(strtrim(name)) {
}
FSNode::FSNode(FSNode *parent, const string &name, const string &path) : parent(parent), type(0), size(0), name(strtrim(name)), path(strtrim(path)) {
}
FSNode::~FSNode() {
	this->clear();
}


bool FSNode::isroot(void) const {
	return false;
}
bool FSNode::isfile(void) const {
	return ((this->type & FS_TYPE_FILE) != 0);
}
bool FSNode::isdirectory(void) const {
	return ((this->type & FS_TYPE_DIRECTORY) != 0);
}
bool FSNode::islocal(void) const {
	return ((this->type & FS_TYPE_LOCAL) != 0);
}
bool FSNode::isremote(void) const {
	return ((this->type & FS_TYPE_REMOTE) != 0);
}
dword FSNode::gettype(void) const {
	return this->type;
}
dword FSNode::getsize(void) const {
	return this->size;
}
const string& FSNode::getname(void) const {
	return this->name;
}
string FSNode::getfullname(void) const {
	if (this->parent != NULL) {
		string pname = this->parent->getfullname();

		if (pname[pname.length() - 1] != '/') {
			return (pname + "/" + this->name);
		}
		return (pname + this->name);
	}
	return ("/" + this->name);
}
const string& FSNode::getpath(void) const {
	return this->path;
}

bool FSNode::hasaccess(const string &user) const {
	if (this->parent != NULL) {
		return this->parent->hasaccess(user);
	}
	return false;
}
void FSNode::mergeusers(const vector<string> &users) {
	if (this->parent != NULL) {
		this->parent->mergeusers(users);
	}
}
vector<string> FSNode::listdirectories(const string &user) const {
	vector<string> items;

	if (this->hasaccess(user)) {
		for (dword i = 0; i < this->children.size(); i++) {
			if (this->children[i]->isroot()) {
				if (this->children[i]->hasaccess(user)) {
					items.push_back(this->children[i]->name);
				}
			} else if (this->children[i]->isdirectory()) {
				items.push_back(this->children[i]->name);
			}
		}
	}
	sort(items.begin(), items.end());
	return items;
}
vector<string> FSNode::listfiles(const string &user) const {
	vector<string> items;

	if (this->hasaccess(user)) {
		for (dword i = 0; i < this->children.size(); i++) {
			if (this->children[i]->isfile()) {
				if (this->children[i]->isroot()) {
					if (this->children[i]->hasaccess(user)) {
						items.push_back(this->children[i]->name);
					}
				} else {
					items.push_back(this->children[i]->name);
				}
			}
		}
	}
	sort(items.begin(), items.end());
	return items;
}

bool FSNode::exists(const string &name) const {
	for (dword i = 0; i < this->children.size(); i++) {
		if (pathcompare(this->children[i]->name, name)) {
			return true;
		}
	}
	return false;
}
FSNode* FSNode::get(const string &name) const {
	for (dword i = 0; i < this->children.size(); i++) {
		if (pathcompare(this->children[i]->name, name)) {
			return this->children[i];
		}
	}
	return NULL;
}
void FSNode::clear(void) {
	for (dword i = 0; i < this->children.size(); i++) {
		delete this->children[i];
	}
	this->children.clear();
}


const FSNode* FSNode::find(const string &path) const {
	vector<string>	items = strsplit(strreplace(strreplace(path, '\\', '/'), "//", "/"), "/", 1);
	dword			i = 0;

	if (items.size() > 0) {
		bool match = false;

		if (items[i].compare("/") == 0) {
			if (this->parent != NULL) {
				return NULL;
			}
			match = true;
			i++;
		}
		while (i < items.size()) {
			if (items[i].compare("/") == 0) {
				i++;
				continue;
			}
			if (pathcompare(items[i], this->name)) {
				match = true;
			} else {
				if (match) {
					for (dword j = 0; j < this->children.size(); j++) {
						const FSNode *p = this->children[j]->find(items[i]);

						if (p != NULL) {
							return p;
						}
					}
				}
				return NULL;
			}
			i++;
		}
		return this;
	}
	return NULL;
}

bool FSNode::lookup(void) {
	if (pathisurl(this->path)) {
		this->type |= FS_TYPE_REMOTE | FS_TYPE_FILE;
		this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_DIRECTORY);
		this->clear();
		DEBUGWRITE("FSNode::lookup() : url=" + this->getfullname());
	} else if (pathisdirectory(this->path)) {
		this->path = pathappendslash(this->path);
		this->type |= FS_TYPE_LOCAL | FS_TYPE_DIRECTORY;
		this->type &= ~(FS_TYPE_REMOTE | FS_TYPE_FILE);
		DEBUGWRITE("FSNode::lookup() : dir=" + this->getfullname());
	} else if (pathisfile(this->path)) {
		this->type |= FS_TYPE_LOCAL | FS_TYPE_FILE;
		this->type &= ~(FS_TYPE_REMOTE | FS_TYPE_DIRECTORY);
		this->clear();
		DEBUGWRITE("FSNode::lookup() : file=" + this->getfullname());
	} else if (this->isroot()) {
		this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_REMOTE | FS_TYPE_DIRECTORY | FS_TYPE_FILE);
		DEBUGWRITE("FSNode::lookup() : root=" + this->getfullname());
	} else {
		this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_REMOTE | FS_TYPE_DIRECTORY | FS_TYPE_FILE);
		this->clear();
		DEBUGWRITE("FSNode::lookup() : none=" + this->getfullname());
		return false;
	}
	return true;
}



FSRoot::FSRoot(FSNode *parent, const string &name, const vector<string> &users) : FSNode(parent, name), users(users), timeout(0.0), recursive(true) {
	if (parent != NULL) {
		parent->mergeusers(users);
	}
	this->lookup();
}
FSRoot::FSRoot(FSNode *parent, const string &name, const vector<string> &users, const string &path) : FSNode(parent, name, path), users(users), timeout(-1), recursive(false) {
	if (parent != NULL) {
		parent->mergeusers(users);
	}
	this->lookup();
}
FSRoot::FSRoot(FSNode *parent, const string &name, const vector<string> &users, const string &path, const vector<string> &exts, double timeout, bool recursive) : FSNode(parent, name, path), users(users), exts(exts), timeout(timeout), recursive(recursive) {
	if (parent != NULL) {
		parent->mergeusers(users);
	}
	this->lookup();
}
FSRoot::~FSRoot() {
}


bool FSRoot::isroot(void) const {
	return true;
}

bool FSRoot::hasaccess(const string &user) const {
	if (this->users.size() > 0) {
		for (dword i = 0; i < this->users.size(); i++) {
			if (this->users[i].compare(user) == 0) {
				return true;
			}
		}
		return false;
	}
	return true;
}
void FSRoot::mergeusers(const vector<string> &users) {
	if (users.size() == 0) {
		this->users.clear();
	}
	if (this->users.size() > 0) {
		for (dword i = 0; i < users.size(); i++) {
			if (!this->hasaccess(users[i])) {
				this->users.push_back(users[i]);
			}
		}
	}
	FSNode::mergeusers(users);
}

void FSRoot::add(FSNode *child) {
	if (this->exists(child->name)) {
		delete child;
	} else {
		this->children.push_back(child);
	}
}
void FSRoot::del(const string &name) {
	for (dword i = 0; i < this->children.size(); i++) {
		if (pathcompare(this->children[i]->name, name)) {
			delete this->children[i];
			this->children.erase(this->children.begin() + i);
			return;
		}
	}
}

void FSRoot::refresh(bool force, FSNode *node) {
	if (force || this->timeout == 0.0 || (this->timeout > 0.0 && this->timer.pick() > this->timeout)) {
		if (pathisdirectory(node->path)) {
			vector<string>	items = getdirectoryitems(node->path, this->exts);
			vector<dword>	remove;

			for (dword i = 0; i < node->children.size(); i++) {
				if (node->children[i]->isroot()) {
					node->children[i]->lookup();
					((FSRoot*)node->children[i])->refresh(force, node->children[i]);
				} else {
					bool r = true;

					if (node->children[i]->lookup()) {
						for (dword j = 0; j < items.size(); j++) {
							if (pathcompare(node->children[i]->name, items[j])) {
								r = false;
								break;
							}
						}
					}
					if (r) {
						remove.push_back(i);
					} else {
						this->refresh(force, node->children[i]);
					}
				}
			}
			if (remove.size() > 0) {
				reverse(remove.begin(), remove.end());
				for (dword i = 0; i < remove.size(); i++) {
					delete node->children[remove[i]];
					node->children.erase(node->children.begin() + remove[i]);
				}
			}
			for (dword i = 0; i < items.size(); i++) {
				bool exists = false;

				for (dword j = 0; j < node->children.size(); j++) {
					if (pathcompare(node->children[j]->name, items[i])) {
						exists = true;
						break;
					}
				}
				if (!exists) {
					string childpath(node->path + items[i]);

					if (this->recursive || pathisfile(childpath) || pathisurl(childpath)) {
						FSNode *child = new FSNode(node, items[i], childpath);

						if (child->lookup()) {
							node->children.push_back(child);
						} else {
							delete child;
						}
					}
				}
			}
		} else if (node->isroot()) {
			for (dword i = 0; i < this->children.size(); i++) {
				if (this->children[i]->isroot()) {
					this->children[i]->lookup();
					((FSRoot*)this->children[i])->refresh(force, this->children[i]);
				} else {
					this->refresh(force, this->children[i]);
				}
			}
		}
		this->timer.start();
	}
}

void FSRoot::refresh(bool force) {
	this->lookup();
	this->refresh(force, this);
}
