// util.cpp
#include "global.h"


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

vector<string> strsplit(const string &value, const string &separator, dword limit) {
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
		items.push_back(string(s));
		last = i + 1;
	}
	return items;
}

vector<string> getdirectoryitems(const string &filename) {
	WIN32_FIND_DATA	wfd;
	vector<string>	items;
	HANDLE			hFind;
	string			separator;

	if (filename[filename.length() - 1] != '\\') {
		separator = "\\";
	}
	hFind = FindFirstFile((filename + separator).c_str(), &wfd);
	if (hFind != INVALID_HANDLE_VALUE) {
		items.push_back(filename + separator + wfd.cFileName);
		while (FindNextFile(hFind, &wfd) != 0) {
			items.push_back(filename + separator + wfd.cFileName);
		}
		FindClose(hFind);
	}
	return items;
}

bool pathisurl(const string &filename) {
	return (filename.find("://") > 0);
}

bool pathisfile(const string &filename) {
	if (PathFileExists(filename.c_str()) == TRUE && PathIsDirectory(filename.c_str()) == FALSE) {
		return true;
	}
	return false;
}

bool pathisdirectory(const string &filename) {
	if (PathFileExists(filename.c_str()) == TRUE && PathIsDirectory(filename.c_str()) == TRUE) {
		return true;
	}
	return false;
}
