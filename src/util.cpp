// util.cpp
#include "global.h"


string strreplace(const string &value, char src, char dst) {
	string ret(value);

	for (dword i = 0; i < ret.length(); i++) {
		if (ret[i] == src) {
			ret[i] = dst;
		}
	}
	return ret;
}
string strreplace(const string &value, const string &src, const string &dst, dword level) {
	string	ret;
	dword	count = 0;

	if (src.length() == 0) {
		return value;
	}
	for (dword i = 0; i < value.length(); i++) {
		if (value[i] == src[0]) {
			bool found = true;

			for (dword j = 1; (i + j) < value.length() && j < src.length(); j++) {
				if (value[i + j] != src[j]) {
					found = false;
					break;
				}
			}
			if (found) {
				ret += dst;
				i += src.length() - 1;
				count++;
				continue;
			}
		}
		ret += value[i];
	}
	if (count > 0 && level < 64) {
		return strreplace(ret, src, dst, level + 1);
	}
	return ret;
}

string strtrim(const string &value) {
	dword i1 = 0, i2 = value.length();

	for (dword i = 0; i < value.length(); i++) {
		if (value[i] != ' ' && value[i] != '\t' && value[i] != '\r' && value[i] != '\n') {
			break;
		}
		i1 = i + 1;
	}
	if (value.length() > 0 && i1 < value.length()) {
		for (dword i = i2 - 1; i2 > i1; i--) {
			if (value[i] != ' ' && value[i] != '\t' && value[i] != '\r' && value[i] != '\n') {
				break;
			}
			i2 = i;
		}
		return value.substr(i1, i2 - i1);
	}
	return string();
}

string strstrip(const string &value, const string &separator) {
	string result;
	dword  last = 0;
	long   i;

	while (last < value.length()) {
		i = -1;
		for (dword j = 0; j < separator.length(); j++) {
			long i2 = value.find(separator[j], last);

			if (i2 >= 0 && (i < 0 || i2 < i)) {
				i = i2;
			}
		}
		if (i < 0) {
			result += value.substr(last, value.length() - last);
			break;
		}
		result += value.substr(last, i - last);
		last = i + 1;
	}
	return result;
}

vector<string> strsplit(const string &value, const string &separator, dword limit, bool withseparator) {
	vector<string>	items;
	dword			last = 0;
	long			i;
	char			s[2];

	while (last < value.length()) {
		i = -1;
		s[0] = s[1] = 0;
		for (dword j = 0; j < separator.length(); j++) {
			long i2 = value.find(separator[j], last);

			if (i2 >= 0 && (i < 0 || i2 < i)) {
				s[0] = separator[j];
				i = i2;
			}
		}
		if (i < 0 || (limit > 0 && items.size() >= limit)) {
			if ((value.length() - last) > 0) {
				items.push_back(value.substr(last, value.length() - last));
			}
			break;
		}
		if ((i - last) > 0) {
			items.push_back(value.substr(last, i - last));
		}
		if (withseparator) {
			items.push_back(string(s));
		}
		last = i + 1;
	}
	return items;
}

bool isvalidfile(const string &path, const string &filename, const vector<string> &exts, bool dir) {
	if (strcmp(filename.c_str(), ".") != 0 && strcmp(filename.c_str(), "..") != 0) {
		if (dir != true && exts.size() > 0) {
			char buffer[MAX_PATH];
			char *ext;

			lstrcpyn(buffer, filename.c_str(), MAX_PATH);
			ext = PathFindExtension(buffer);
			for (dword i = 0; i < exts.size(); i++) {
				if (exts[i].compare(ext) == 0) {
					return true;
				}
			}
			return false;
		}
		return true;
	}
	return false;
}

vector<string> getdirectoryitems(const string &filename, const vector<string> &exts) {
	WIN32_FIND_DATA	wfd;
	vector<string>	items;
	HANDLE			hFind;
	string			path = pathappendslash(filename);

	hFind = FindFirstFile((path + "*").c_str(), &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		if (isvalidfile(path, wfd.cFileName, exts, ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ? true : false)) {
			items.push_back(wfd.cFileName);
		}
		while (FindNextFile(hFind, &wfd) != 0) {
			if (isvalidfile(path, wfd.cFileName, exts, ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ? true : false)) {
				items.push_back(wfd.cFileName);
			}
		}
		FindClose(hFind);
	}
	sort(items.begin(), items.end());
	return items;
}

bool pathexists(const string &filename) {
	if (PathFileExists(filename.c_str())) {
		return true;
	}
	return false;
}

bool pathcompare(const string &src1, const string &src2) {
	return (strcmpi(src1.c_str(), src2.c_str()) == 0);
}

bool pathisurl(const string &filename) {
	if (filename.find("://") < filename.length()) {
		return true;
	}
	return false;
}

bool pathisfile(const string &filename) {
	if (PathFileExists(filename.c_str()) && !PathIsDirectory(filename.c_str())) {
		return true;
	}
	return false;
}

bool pathisdirectory(const string &filename) {
	if (PathFileExists(filename.c_str()) && PathIsDirectory(filename.c_str())) {
		return true;
	}
	return false;
}

string pathappendslash(const string &filename) {
	if (filename[filename.length() - 1] != '\\') {
		return (filename + "\\");
	}
	return filename;
}

#ifdef _DEBUG

void debugclear() {
	CloseHandle(CreateFile("C:\\ngwinamp.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, 0, NULL));
}

void debugwrite(const char *text) {
	HANDLE	hFile = CreateFile("C:\\ngwinamp.log", GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, 0, NULL);
	DWORD	bw;

	SetFilePointer(hFile, 0, NULL, FILE_END);
	WriteFile(hFile, text, strlen(text), &bw, NULL);
	WriteFile(hFile, "\r\n", 2, &bw, NULL);
	CloseHandle(hFile);
}

void debugwrite(const string &text) {
	debugwrite(text.c_str());
}

#endif
