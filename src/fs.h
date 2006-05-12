// fs.h
#ifndef _FS_H_INCLUDE_
#define _FS_H_INCLUDE_


#define FS_TYPE_NONE		0x00000000
#define FS_TYPE_LOCAL		0x00000002
#define FS_TYPE_REMOTE		0x00000004
#define FS_TYPE_FILE		0x00000010
#define FS_TYPE_DIRECTORY	0x00000020


class FSRoot;


class FSNode {
protected:
	friend class FSRoot;

	FSNode			*parent;
	vector<FSNode*>	children;
	dword			type;
	dword			size;
	string			name;
	string			path;

	bool lookup(void);


public:
	FSNode(FSNode *parent, const string &name);
	FSNode(FSNode *parent, const string &name, const string &path);
	virtual ~FSNode();

	virtual bool isroot(void) const;
	bool isfile(void) const;
	bool isdirectory(void) const;
	bool islocal(void) const;
	bool isremote(void) const;
	dword gettype(void) const;
	dword getsize(void) const;
	const string& getname(void) const;
	string getfullname(void) const;
	const string& getpath(void) const;

	virtual bool hasaccess(const string &user) const;
	virtual void mergeusers(const vector<string> &users);
	vector<string> listdirectories(const string &user) const;
	vector<string> listfiles(const string &user) const;

	bool exists(const string &name) const;
	FSNode* get(const string &name) const;
	void clear(void);

	const FSNode* find(const string &path) const;
};


class FSRoot : public FSNode {
protected:
	vector<string>	exts;
	vector<string>	users;
	NGTIMER			timer;
	double			timeout;
	bool			recursive;

	void refresh(bool force, FSNode *node);


public:
	FSRoot(FSNode *parent, const string &name, const vector<string> &users);
	FSRoot(FSNode *parent, const string &name, const vector<string> &users, const string &path);
	FSRoot(FSNode *parent, const string &name, const vector<string> &users, const string &path, const vector<string> &exts, double refresh, bool recursive);
	virtual ~FSRoot();

	bool isroot(void) const;

	bool hasaccess(const string &user) const;
	void mergeusers(const vector<string> &users);

	void add(FSNode *child);
	void del(const string &name);


	void refresh(bool force);
};


#endif //_FS_H_INCLUDE_
