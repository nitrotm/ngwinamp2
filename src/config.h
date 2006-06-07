// config.h
#ifndef _CONFIG_H_INCLUDE_
#define _CONFIG_H_INCLUDE_


/**
  * Configuration node
  *
  */
class CFGNode {
private:
	string	name;
	string	value;
	bool	group;

	vector<CFGNode>	children;


public:
	CFGNode(bool group, const string &name);
	CFGNode(const string &name, const string &value);
	CFGNode(const CFGNode &src);

	bool isgroup(void) const;
	const string& getname(void) const;
	const string& getstr(void) const;
	bool getbool(void) const;
	int getint(void) const;
	dword getuint(void) const;
	double getfloat(void) const;


	dword size(void) const;
	bool exists(const string &name) const;
	CFGNode getchild(const string &name) const;
	CFGNode getchild(dword i) const;
	void addchild(const CFGNode &node);
	void delchild(dword i);
	void free(void);
};


/**
  * Configuration manager
  *
  */
class CFG {
private:
	CFGNode root;


	dword parsegroup(CFGNode &node, const string &name, const vector<string> &tokens, dword index);

	void writenode(HANDLE hFile, CFGNode &node, const string &indent);


public:
	CFG();

	CFGNode get(const string &name);
	void free(void);

	void read(const string &filename);
	void write(const string &filename);
};


#endif //_CONFIG_H_INCLUDE_
