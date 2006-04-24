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


public:
	FSNode() : type(FS_TYPE_NONE), size(0) {
	}
	FSNode(const string &path) : type(0), size(0), path(path) {
		this->refresh();
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


	void refresh(void) {
		if (pathisurl(this->path)) {
			this->type |= FS_TYPE_REMOTE | FS_TYPE_FILE;
			this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_DIRECTORY);
		} else if (pathisdirectory(this->path)) {
			this->type |= FS_TYPE_LOCAL | FS_TYPE_DIRECTORY;
			this->type &= ~(FS_TYPE_REMOTE | FS_TYPE_FILE);
		} else if (pathisfile(this->path)) {
			this->type |= FS_TYPE_LOCAL | FS_TYPE_FILE;
			this->type &= ~(FS_TYPE_REMOTE | FS_TYPE_DIRECTORY);
		} else {
			this->type &= ~(FS_TYPE_LOCAL | FS_TYPE_REMOTE | FS_TYPE_DIRECTORY | FS_TYPE_FILE);
		}
		if (this->islocal() && this->isdirectory()) {
			vector<string> items = getdirectoryitems(this->path.c_str());
			vector<dword>  remove;

			for (dword i = 0; i < this->children.size(); i++) {
				bool r = true;

				for (dword j = 0; j < items.size(); j++) {
					if (this->children[i].path.compare(items[j]) == 0) {
						r = false;
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
					this->children.push_back(FSNode(items[i]));
				}
			}
		}
	}
};


#endif //_FS_H_INCLUDE_
