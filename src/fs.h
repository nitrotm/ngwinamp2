// fs.h
#ifndef _FS_H_INCLUDE_
#define _FS_H_INCLUDE_


#define FS_TYPE_NONE		0x00000000
#define FS_TYPE_ROOT		0x00000001
#define FS_TYPE_LOCAL		0x00000002
#define FS_TYPE_REMOTE		0x00000004
#define FS_TYPE_FILE		0x00000010
#define FS_TYPE_DIRECTORY	0x00000020


class FSNode {
protected:
	dword			type;
	dword			size;
	string			name;
	string			path;

	vector<FSNode>	children;
	vector<string>	exts;


public:
	FSNode() : type(FS_TYPE_NONE), size(0) {
	}
	FSNode(const string &path, const vector<string> &exts) : type(0), size(0), path(path) {
		this->refresh(exts);
	}
	FSNode(const string &path, const vector<string> &exts, bool root) : type(0), size(0), path(path) {
		if (root) {
			this->type |= FS_TYPE_ROOT;
			this->exts = exts;
		}
		this->refresh(exts);
	}
	FSNode(const FSNode &src) : type(src.type), size(src.size), name(src.name), path(src.path), children(src.children) {
	}


	bool isroot(void) const {
		return ((this->type & FS_TYPE_ROOT) != 0);
	}
	bool isfile(void) const {
		return ((this->type & FS_TYPE_FILE) != 0);
	}
	bool isdirectory(void) const {
		return ((this->type & FS_TYPE_DIRECTORY) != 0);
	}
	bool islocal(void) const {
		return ((this->type & FS_TYPE_LOCAL) != 0);
	}
	bool isremote(void) const {
		return ((this->type & FS_TYPE_REMOTE) != 0);
	}
	dword gettype(void) const {
		return this->type;
	}
	dword getsize(void) const {
		return this->size;
	}
	string getname(void) const {
		return this->name;
	}
	string getpath(void) const {
		return this->path;
	}

	bool exists(const string &name) const {
		for (dword i = 0; i < this->children.size(); i++) {
			if (this->children[i].name.compare(name) == 0) {
				return true;
			}
		}
		return false;
	}
	void add(const FSNode &child) {
		if (!this->exists(child.name)) {
			this->children.push_back(child);
		}
	}
	FSNode get(const string &name) const {
		for (dword i = 0; i < this->children.size(); i++) {
			if (this->children[i].name.compare(name) == 0) {
				return this->children[i];
			}
		}
		return FSNode();
	}
	void del(const string &name) {
		for (dword i = 0; i < this->children.size(); i++) {
			if (this->children[i].name.compare(name) == 0) {
				this->children.erase(this->children.begin() + i);
				return;
			}
		}
	}


	FSNode find(const string &path) const {
		// split next "/"
		return FSNode();
	}


	bool refresh(void) {
		if (this->isroot()) {
			return this->refresh(this->exts);
		}
		return false;
	}
	bool refresh(const vector<string> &exts) {
		if (pathisurl(this->path)) {
			this->type |= FS_TYPE_REMOTE | FS_TYPE_FILE;
			this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_DIRECTORY);
			DEBUGWRITE(("FSNode::refresh() : url[" + this->path + "]").c_str());
		} else if (pathisdirectory(this->path)) {
			vector<string> items = getdirectoryitems(this->path.c_str(), exts);
			vector<dword>  remove;

			this->type |= FS_TYPE_LOCAL | FS_TYPE_DIRECTORY;
			this->type &= ~(FS_TYPE_REMOTE | FS_TYPE_FILE);
			DEBUGWRITE(("FSNode::refresh() : dir[" + this->path + "]").c_str());

			for (dword i = 0; i < this->children.size(); i++) {
				bool r = true;

				for (dword j = 0; j < items.size(); j++) {
					if (this->children[i].path.compare(items[j]) == 0) {
						if (this->children[i].isroot() || this->children[i].refresh(this->isroot() ? this->exts : exts)) {
							r = false;
						}
						break;
					}
				}
				if (r) {
					remove.push_back(i);
				}
			}
			if (remove.size() > 0) {
				for (dword i = remove.size() - 1; i > 0; i--) {
					this->children.erase(this->children.begin() + remove[i]);
				}
			}
			for (dword i = 0; i < items.size(); i++) {
				bool exists = false;

				for (dword j = 0; j < this->children.size(); j++) {
					if (this->children[j].path.compare(items[i]) == 0) {
						exists = true;
						break;
					}
				}
				if (!exists) {
					this->children.push_back(FSNode(items[i], exts));
				}
			}
		} else if (pathisfile(this->path)) {
			this->type |= FS_TYPE_LOCAL | FS_TYPE_FILE;
			this->type &= ~(FS_TYPE_REMOTE | FS_TYPE_DIRECTORY);
			DEBUGWRITE(("FSNode::refresh() : file[" + this->path + "]").c_str());
		} else {
			this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_REMOTE | FS_TYPE_DIRECTORY | FS_TYPE_FILE);
			DEBUGWRITE(("FSNode::refresh() : none[" + this->path + "]").c_str());
			return false;
		}
		return true;
	}
};


#endif //_FS_H_INCLUDE_
