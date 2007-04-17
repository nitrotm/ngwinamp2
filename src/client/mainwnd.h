// mainwnd.h
#ifndef _MAINWND_H_INCLUDE_
#define _MAINWND_H_INCLUDE_


/**
  * Cursors
  *
  */
#define CURSOR_MAX_COUNT	0x05
#define CURSOR_NORMAL		0x00
#define CURSOR_HRESIZE		0x01
#define CURSOR_VRESIZE		0x02
#define CURSOR_ADD			0x03
#define CURSOR_NO			0x04

/**
  * Icons
  *
  */
#define ICON_MAX_COUNT		0x01
#define ICON_MAIN			0x00

/**
  * Toolboxes
  *
  */
#define TOOLBOX_MAX_COUNT	0x04
#define TOOLBOX_CONNECT		0x00
#define TOOLBOX_CONNECTING	0x01
#define TOOLBOX_ADMIN		0x02
#define TOOLBOX_USER		0x03

/**
  * Custom control IDs
  *
  */
#define IDC_STATUSBAR		1000

/**
  * A playlist item
  *
  */
class PlayListItem {
public:
	string	name;
	string	path;

	PlayListItem(const string &name, const string &path) : name(name), path(path) {
	}
	PlayListItem(const PlayListItem &ref) : name(ref.name), path(ref.path) {
	}
};

/**
  * A file item
  *
  */
class FileItem {
public:
	string	name;
	dword	type;
	dword	size;

	FileItem(const string &name, const dword type, const dword size) : name(name), type(type), size(size) {
	}
	FileItem(const FileItem &ref) : name(ref.name), type(ref.type), size(ref.size) {
	}
};

/**
  * A path in the tree
  *
  */
class PathNode {
public:
	HTREEITEM			hparent;
	HTREEITEM			hnode;
	string				path;
	dword				type;

	vector<FileItem>	files;
	bool				filesloaded;

	PathNode(HTREEITEM hparent, HTREEITEM hnode, const string &path, const dword type) : hparent(hparent), hnode(hnode), path(path), type(type), filesloaded(false) {
	}
	PathNode(const PathNode &ref) : hparent(ref.hparent), hnode(ref.hnode), path(ref.path), type(ref.type), files(ref.files), filesloaded(ref.filesloaded) {
	}
};

/**
  * A generic drag'n drop operation
  *
  */
class DragNDrop {
public:
	DragNDrop() {
	}

	virtual bool isfilelist() {
		return false;
	}
	virtual bool isplaylist() {
		return false;
	}
};

/**
  * A filelist drag'n drop operation
  *
  */
class FileDragNDrop : public DragNDrop {
public:
	vector<string> filelist;
	long index;

	FileDragNDrop(const vector<string> &filelist) : DragNDrop(), filelist(filelist), index(-1) {
	}

	bool isfilelist() {
		return true;
	}
};

/**
  * A playlist drag'n drop operation
  *
  */
class PlayDragNDrop : public DragNDrop {
public:
	vector<dword> indexes;
	long index;

	PlayDragNDrop(const vector<dword> &indexes) : DragNDrop(), indexes(indexes), index(-1) {
	}

	bool isplaylist() {
		return true;
	}
};

/**
  * Main window controller
  *
  */
class NGMainWnd : public NGLOCK {
protected:
	// global
	HINSTANCE	hinstance;
	HCURSOR		cursors[CURSOR_MAX_COUNT];
	HICON		icons[ICON_MAX_COUNT];
	HIMAGELIST	himages;
	// mainwnd
	WNDCLASSEX	wndclass;
	ATOM		hclass;
	HWND		hwnd;
	HMENU		hmenu;
	int			x;
	int			y;
	dword		width;
	dword		height;
	dword		clientwidth;
	dword		clientheight;
	// statusbar
	HWND		hstatuswnd;
	dword		statuswidth;
	dword		statusheight;
	// toolbox
	HWND		htoolboxwnd[TOOLBOX_MAX_COUNT];
	dword		toolboxwidth[TOOLBOX_MAX_COUNT];
	dword		toolboxheight[TOOLBOX_MAX_COUNT];
	dword		toolboxminwidth;
	int			curtoolbox;
	// horizontal/vertical bar
	bool		hresize;
	bool		vresize;
	dword		lastmx;
	dword		lastmy;
	// browsewnd
	HWND		hbrowsewnd;
	dword		browsewidth;
	dword		browseheight;
	vector<PathNode> browsenodes;
	long		browseindex;
	// filelistwnd
	HWND		hfilewnd;
	dword		filewidth;
	dword		fileheight;
	// playlistwnd
	HWND		hplwnd;
	dword		plwidth;
	dword		plheight;
	vector<PlayListItem> playlist;
	long		playlist_pos;
	dword		playlist_size;

	// drag'n drop operations
	DragNDrop	*dragndrop;

	// network client
	NGWINAMPCLIENT *client;


public:
	NGMainWnd(HINSTANCE hinstance);
	virtual ~NGMainWnd();


	bool init(void);
	bool free(void);

	bool create(int x, int y, dword width, dword height);
	bool destroy(void);

	bool main(void);

	// window events
	bool onwnd_created(HWND hwnd);
	bool onwnd_close(void);
	bool onwnd_sizing(dword &width, dword &height);
	bool onwnd_resized(void);
	bool onwnd_mousemove(dword mx, dword my, bool mousedown);
	bool onwnd_command(WPARAM nc, WPARAM id);
	bool onwnd_notify(WPARAM id, LPNMHDR hdr);
	bool onwnd_hscroll(dword code, dword position, HWND hcontrol);

	// network events
	bool onnet_connected(bool success, const string &title, const string &message);
	bool onnet_disconnect(void);
	bool onnet_authfailed(dword code);
	bool onnet_authsuccess(const NETAUTHEX &auth);
	bool onnet_setvolume(double volume);
	bool onnet_setpan(double pan);
	bool onnet_setposition(double progress, dword current, dword length);
	bool onnet_setshuffle(bool shuffle);
	bool onnet_setrepeat(bool repeat);
	bool onnet_directories(const string &path, const vector<string> &directories, const vector<dword> &types, const vector<dword> &childrens);
	bool onnet_files(const string &path, const vector<string> &files, const vector<dword> &types, const vector<dword> &sizes);
	bool onnet_playlistpos(const dword index, const dword size);
	bool onnet_playlistnames(const vector<string> &names, const vector<dword> &indexes, const dword total);
	bool onnet_playlistfiles(const vector<string> &files, const vector<dword> &indexes, const dword total);

	// gui utilities
	bool isDragNDrop();
	void refreshplaylistpos();
	void listview_refreshplaylist();
	void listview_setfiles(const vector<FileItem> &files);
	void toolbox_select(int id);
	static string textbox_getstring(HWND hwnd, int id);
	static bool checkbox_selected(HWND hwnd, int id);
	static bool menuitem_checked(HMENU hmenu, int id);
	static vector<dword> listview_getselection(HWND hwnd);
};



#endif //_MAINWND_H_INCLUDE_
