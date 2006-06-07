// config.cpp
#include "global.h"
#include "util.h"
#include "config.h"


CFGNode::CFGNode(bool group, const string &name) : group(group), name(name) {
}
CFGNode::CFGNode(const string &name, const string &value) : group(false), name(name), value(value) {
}
CFGNode::CFGNode(const CFGNode &src) : group(src.group), name(src.name), value(src.value), children(src.children) {
}


bool CFGNode::isgroup(void) const {
	return this->group;
}

const string& CFGNode::getname(void) const {
	return this->name;
}

const string& CFGNode::getstr(void) const {
	return this->value;
}
bool CFGNode::getbool(void) const {
	return (this->value.compare("true") == 0);
}
int CFGNode::getint(void) const {
	return atoi(this->value.c_str());
}
dword CFGNode::getuint(void) const {
	return (dword)atol(this->value.c_str());
}
double CFGNode::getfloat(void) const {
	return atof(this->value.c_str());
}

dword CFGNode::size(void) const {
	return this->children.size();
}
bool CFGNode::exists(const string &name) const {
	for (dword i = 0; i < this->children.size(); i++) {
		if (this->children[i].name.compare(name) == 0) {
			return true;
		}
	}
	return false;
}
CFGNode CFGNode::getchild(const string &name) const {
	for (dword i = 0; i < this->children.size(); i++) {
		if (this->children[i].name.compare(name) == 0) {
			return this->children[i];
		}
	}
	return CFGNode(false, "");
}
CFGNode CFGNode::getchild(dword i) const {
	if (i < this->children.size()) {
		return this->children[i];
	}
	return CFGNode(false, "");
}
void CFGNode::addchild(const CFGNode &node) {
	if (this->group) {
		this->children.push_back(node);
	}
}
void CFGNode::delchild(dword i) {
	if (i < this->children.size()) {
		this->children.erase(this->children.begin() + i);
	}
}
void CFGNode::free(void) {
	this->children.clear();
}


CFG::CFG() : root(true, "") {
}


CFGNode CFG::get(const string &name) {
	return this->root.getchild(name);
}
void CFG::free(void) {
	this->root.free();
}

void CFG::read(const string &filename) {
	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	this->free();
	if (hFile != INVALID_HANDLE_VALUE) {
		vector<string>	tokens;
		string			value;
		DWORD			br, size, offset = 0;
		char			buffer[2048];

		size = GetFileSize(hFile, NULL);
		while (offset < size) {
			ReadFile(hFile, buffer, sizeof(buffer) - 1, &br, NULL);
			buffer[br] = 0;
			value += buffer;
			offset += br;
		}

		tokens = strsplit(value, "{};", 0);
		for (long i = 0; i < (long)tokens.size(); i++) {
			tokens[i] = strtrim(tokens[i]);
			if (tokens[i].length() == 0) {
				tokens.erase(tokens.begin() + i);
				i--;
			}
		}
		for (dword i = 0; i < tokens.size(); i++) {
			if (i > 0 && tokens[i][0] == '{') {
				i = this->parsegroup(this->root, tokens[i - 1], tokens, i + 1);
			}
		}
		CloseHandle(hFile);
	}
}
void CFG::write(const string &filename) {
	HANDLE hFile = CreateFile(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL);

	if (hFile != INVALID_HANDLE_VALUE) {
		for (dword i = 0; i < this->root.size(); i++) {
			this->writenode(hFile, this->root.getchild(i), "");
		}
		CloseHandle(hFile);
	}
}

dword CFG::parsegroup(CFGNode &node, const string &name, const vector<string> &tokens, dword index) {
	CFGNode cur(true, name);
	dword   ret = tokens.size();

	for (dword i = index; i < ret; i++) {
		vector<string> pair;

		// sub group
		if (tokens[i][0] == '{') {
			if (i > 0) {
				i = this->parsegroup(cur, tokens[i - 1], tokens, i + 1);
			} else {
				// syntax error !
				break;
			}
		}
		if (tokens[i][0] == '}') {
			ret = i + 1;
			break;
		}

		// key = value ?
		pair = strsplit(tokens[i], "=", 1);
		if (pair.size() == 3) {
			cur.addchild(CFGNode(strtrim(pair[0]), strtrim(pair[2])));
		} else if (pair.size() == 2) {
			cur.addchild(CFGNode(strtrim(pair[0]), ""));
		}
	}
	node.addchild(cur);
	return ret;
}
void CFG::writenode(HANDLE hFile, CFGNode &node, const string &indent) {
	DWORD bw;

	if (node.size() > 0) {
		WriteFile(hFile, indent.c_str(), indent.length(), &bw, NULL);
		WriteFile(hFile, node.getname().c_str(), node.getname().length(), &bw, NULL);
		WriteFile(hFile, " {\r\n", 4, &bw, NULL);
		for (dword i = 0; i < node.size(); i++) {
			this->writenode(hFile, node.getchild(i), indent + "\t");
		}
		WriteFile(hFile, indent.c_str(), indent.length(), &bw, NULL);
		WriteFile(hFile, "}\r\n", 3, &bw, NULL);
	} else {
		WriteFile(hFile, indent.c_str(), indent.length(), &bw, NULL);
		WriteFile(hFile, node.getname().c_str(), node.getname().length(), &bw, NULL);
		WriteFile(hFile, " = ", 3, &bw, NULL);
		if (node.getstr().length() > 0) {
			WriteFile(hFile, node.getstr().c_str(), node.getstr().length(), &bw, NULL);
		}
		WriteFile(hFile, ";\r\n", 3, &bw, NULL);
	}
}
