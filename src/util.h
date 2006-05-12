// global.h
#ifndef _UTIL_H_INCLUDE_
#define _UTIL_H_INCLUDE_


// zzip library
#include "zzip/zziplib.h"


/**
  * Timer object
  *
  */
class NGTIMER {
private:
	LARGE_INTEGER	frequency;
	LARGE_INTEGER	s_time;
	LARGE_INTEGER	l_time;
	__int64			elapsed;
	__int64			delta;
	double			sdelta;
	double			selapsed;


public:
	NGTIMER() {
		QueryPerformanceFrequency(&this->frequency);
		QueryPerformanceCounter(&this->s_time);
		memcpy(&this->l_time, &this->s_time, sizeof(LARGE_INTEGER));
	}
	NGTIMER(const NGTIMER &src) : elapsed(src.elapsed), delta(src.delta), sdelta(src.sdelta), selapsed(src.selapsed) {
		memcpy(&this->frequency, &src.frequency, sizeof(LARGE_INTEGER));
		memcpy(&this->s_time, &src.s_time, sizeof(LARGE_INTEGER));
		memcpy(&this->l_time, &src.l_time, sizeof(LARGE_INTEGER));
	}

	void start() {
		QueryPerformanceCounter(&this->s_time);
		memcpy(&this->l_time, &this->s_time, sizeof(LARGE_INTEGER));
	}
	double pick() {
		LARGE_INTEGER current;

		QueryPerformanceCounter(&current);
		this->elapsed = current.QuadPart - this->s_time.QuadPart;
		this->delta = current.QuadPart - this->l_time.QuadPart;
		if (this->frequency.QuadPart > 0) {
			this->selapsed = (double)this->elapsed / (double)(this->frequency.QuadPart);
			this->sdelta = (double)this->delta / (double)(this->frequency.QuadPart);
		}
		return this->sdelta;
	}
	double getdelta() {
		return this->sdelta;
	}
	double getelapsed() {
		return this->selapsed;
	}
};


/**
  * Global lock wrapper
  *
  */
class NGLOCK {
private:
	CRITICAL_SECTION l;
	dword			 count;

public:
	NGLOCK() : count(0) {
		InitializeCriticalSection(&this->l);
	}
	NGLOCK(const NGLOCK &ref) : count(0) {
		InitializeCriticalSection(&this->l);
		for (dword i = 0; i < ref.count; i++) {
			this->lock();
		}
	}
	virtual ~NGLOCK() {
		while (this->count > 0) {
			this->unlock();
		}
		DeleteCriticalSection(&this->l);
	}

	void lock() {
		EnterCriticalSection(&this->l);
		this->count++;
	}
	void unlock() {
		if (this->count > 0) {
			this->count--;
			LeaveCriticalSection(&this->l);
		}
	}
};

/**
  * Local lock wrapper
  *
  */
class NGLOCKER {
protected:
	NGLOCK	*lock;
	dword	 count;

public:
	NGLOCKER(NGLOCK *lock) : lock(lock), count(0) {
		this->acquire();
	}
	virtual ~NGLOCKER() {
		while (count > 0) {
			this->release();
		}
	}

	void acquire() {
		this->lock->lock();
		this->count++;
	}
	void release() {
		if (this->count > 0) {
			this->count--;
			this->lock->unlock();
		}
	}
};


/**
  * Buffer wrapper
  *
  */
class NGBUFFER {
protected:
	dword	bsize;
	byte	*ptr;


public:
	NGBUFFER() : bsize(0), ptr(NULL) {
	}
	NGBUFFER(dword size) : bsize(0), ptr(NULL) {
		this->alloc(size);
	}
	NGBUFFER(const void *ptr, dword size) : bsize(0), ptr(NULL) {
		this->set(ptr, size);
	}
	NGBUFFER(const NGBUFFER &src) : bsize(0), ptr(NULL) {
		this->set(src);
	}
	virtual ~NGBUFFER() {
		this->free();
	}

	void alloc(dword size) {
		this->free();
		this->bsize = size;
		this->ptr = new byte[size];
	}
	void free() {
		if (this->ptr != NULL) {
			delete [] this->ptr;
			this->ptr = NULL;
		}
		this->bsize = 0;
	}

	const NGBUFFER get(dword offset, dword length) const {
		if (offset < this->bsize) {
			if (offset + length > this->bsize) {
				length = this->bsize - offset;
			}
			return NGBUFFER(this->ptr + offset, length);
		}
		return NGBUFFER();
	}
	string tostring(dword offset, dword length) const {
		if (offset < this->bsize) {
			if (offset + length > this->bsize) {
				length = this->bsize - offset;
			}
			if (length > 0) {
				string value;
				char   *buffer = new char[length + 1];

				memcpy(buffer, this->ptr + offset, length);
				buffer[length] = 0;
				value = buffer;
				delete [] buffer;
				return value;
			}
		}
		return string();
	}
	string tostring() const {
		return this->tostring(0, this->bsize);
	}

	dword size(void) const {
		return this->bsize;
	}
	dword read(void *ptr, dword offset, dword length) const {
		if (offset < this->bsize) {
			if (offset + length > this->bsize) {
				length = this->bsize - offset;
			}
			memcpy(ptr, this->ptr + offset, length);
			return length;
		}
		return 0;
	}
	dword write(const void *ptr, dword offset, dword length) {
		if (offset < this->bsize) {
			if (offset + length > this->bsize) {
				length = this->bsize - offset;
			}
			if (length > 0) {
				memcpy(this->ptr + offset, ptr, length);
			}
			return length;
		}
		return 0;
	}
	dword write(const NGBUFFER &src, dword offset, dword length) {
		if (length > src.bsize) {
			length = src.bsize;
		}
		return this->write(src.ptr, offset, length);
	}

	void append(const void *ptr, dword size) {
		byte *tmp = new byte[this->bsize + size];

		if (this->bsize > 0) {
			memcpy(tmp, this->ptr, this->bsize);
		}
		if (size > 0) {
			memcpy(tmp + this->bsize, ptr, size);
		}
		delete [] this->ptr;
		this->ptr = tmp;
		this->bsize += size;
	}
	void append(const NGBUFFER &src) {
		this->append(src.ptr, src.bsize);
	}
	void erase(dword offset, dword length) {
		if (offset < this->bsize && length > 0) {
			if (offset + length > this->bsize) {
				length = this->bsize - offset;
			}

			byte *tmp = new byte[this->bsize - length];

			if (offset > 0) {
				memcpy(tmp, this->ptr, offset);
			}
			if (offset + length < this->bsize) {
				memcpy(tmp + offset, this->ptr + offset + length, this->bsize - offset - length);
			}
			delete [] this->ptr;
			this->ptr = tmp;
			this->bsize -= length;
		}
	}
	void set(const void *ptr, dword size) {
		this->alloc(size);
		if (size > 0) {
			memcpy(this->ptr, ptr, size);
		}
	}
	void set(const NGBUFFER &src) {
		this->set(src.ptr, src.bsize);
	}
};


// String utilities
string strreplace(const string &value, char src, char dst);
string strreplace(const string &value, const string &src, const string &dst, dword level = 0);
string strtrim(const string &value);
string strstrip(const string &value, const string &separator);
vector<string> strsplit(const string &value, const string &separator, dword limit, bool withseparator = true);

// Path utilities
vector<string> getdirectoryitems(const string &filename, const vector<string> &exts);
string pathappendslash(const string &filename);
bool pathexists(const string &filename);
bool pathcompare(const string &src1, const string &src2);
bool pathisurl(const string &filename);
bool pathisfile(const string &filename);
bool pathisdirectory(const string &filename);


// Debug utilities
#ifdef _DEBUG
void debugclear();
void debugwrite(const char *text);
void debugwrite(const string &text);

#define DEBUGCLEAR			debugclear()
#define DEBUGWRITE(text)	debugwrite(text)

#else

#define DEBUGCLEAR			
#define DEBUGWRITE(text)	

#endif


#endif //_UTIL_H_INCLUDE_
