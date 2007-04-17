// global.h
#ifndef _UTIL_H_INCLUDE_
#define _UTIL_H_INCLUDE_


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

	void start(void) {
		QueryPerformanceCounter(&this->s_time);
		memcpy(&this->l_time, &this->s_time, sizeof(LARGE_INTEGER));
	}
	double pick(void) {
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
	double getdelta(void) {
		return this->sdelta;
	}
	double getelapsed(void) {
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

	void lock(void) {
		EnterCriticalSection(&this->l);
		this->count++;
	}
	void unlock(void) {
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

	void acquire(void) {
		this->lock->lock();
		this->count++;
	}
	void release(void) {
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
	void free(void) {
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
	string tostring(void) const {
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


/**
 * Buffer reader
 *
 */
class NGREADER {
protected:
	NGBUFFER	buffer;
	dword		offset;
	dword		error;

public:
	NGREADER(const NGBUFFER &buffer) : buffer(buffer), offset(0), error(0) {
	}

	bool isError(void) const {
		return (this->error > 0);
	}

	bool readBool(void) {
		byte value = 0;

		if (!this->isError()) {
			if (this->buffer.read(&value, this->offset, sizeof(byte)) == sizeof(byte)) {
				this->offset += sizeof(byte);
			} else {
				this->error++;
			}
		}
		return (value != 0x00);
	}

	byte readByte(void) {
		byte value = 0;

		if (!this->isError()) {
			if (this->buffer.read(&value, this->offset, sizeof(byte)) == sizeof(byte)) {
				this->offset += sizeof(byte);
			} else {
				this->error++;
			}
		}
		return value;
	}

	word readWord(void) {
		word value = 0;

		if (!this->isError()) {
			if (this->buffer.read(&value, this->offset, sizeof(word)) == sizeof(word)) {
				this->offset += sizeof(word);
			} else {
				this->error++;
			}
		}
		return value;
	}

	dword readDword(void) {
		dword value = 0;

		if (!this->isError()) {
			if (this->buffer.read(&value, this->offset, sizeof(dword)) == sizeof(dword)) {
				this->offset += sizeof(dword);
			} else {
				this->error++;
			}
		}
		return value;
	}

	float readFloat(void) {
		float value = 0;

		if (!this->isError()) {
			if (this->buffer.read(&value, this->offset, sizeof(float)) == sizeof(float)) {
				this->offset += sizeof(float);
			} else {
				this->error++;
			}
		}
		return value;
	}

	double readDouble(void) {
		double value = 0;

		if (!this->isError()) {
			if (this->buffer.read(&value, this->offset, sizeof(double)) == sizeof(double)) {
				this->offset += sizeof(double);
			} else {
				this->error++;
			}
		}
		return value;
	}

	string readByteString(void) {
		string value;

		if (!this->isError()) {
			byte length = this->readByte();

			value = this->buffer.tostring(this->offset, length);
			if (value.length() == length) {
				this->offset += length;
			} else {
				this->error++;
			}
		}
		return value;
	}

	string readWordString(void) {
		string value;

		if (!this->isError()) {
			word length = this->readWord();

			value = this->buffer.tostring(this->offset, length);
			if (value.length() == length) {
				this->offset += length;
			} else {
				this->error++;
			}
		}
		return value;
	}

	string readDwordString(void) {
		string value;

		if (!this->isError()) {
			dword length = this->readDword();

			value = this->buffer.tostring(this->offset, length);
			if (value.length() == length) {
				this->offset += length;
			} else {
				this->error++;
			}
		}
		return value;
	}

	void read(void *ptr, const dword size) {
		if (!this->isError()) {
			if (this->buffer.read(ptr, this->offset, size) == size) {
				this->offset += size;
			} else {
				this->error++;
			}
		}
	}

	void skip(const dword size) {
		if (!this->isError()) {
			if ((this->offset + size) <= this->buffer.size()) {
				this->offset += size;
			} else {
				this->error++;
			}
		}
	}
};


/**
 * Buffer writer
 *
 */
class NGWRITER {
protected:
	NGBUFFER buffer;

public:
	NGWRITER() {
	}

	operator const NGBUFFER &(void) const {
		return this->buffer;
	}

	dword getsize(void) const {
		return buffer.size();
	}

	void writeBool(const bool value) {
		byte raw;

		if (value) {
			raw = 0xFF;
		} else {
			raw = 0x00;
		}
		this->buffer.append(&raw, sizeof(byte));
	}

	void writeByte(const byte value) {
		this->buffer.append(&value, sizeof(byte));
	}

	void writeWord(const word value) {
		this->buffer.append(&value, sizeof(word));
	}

	void writeDword(const dword value) {
		this->buffer.append(&value, sizeof(dword));
	}

	void writeFloat(const float value) {
		this->buffer.append(&value, sizeof(float));
	}

	void writeDouble(const double value) {
		this->buffer.append(&value, sizeof(double));
	}

	void writeByteString(const string &value) {
		string final = value.length() > 0xFF ? value.substr(0, 0xFF) : value;
		byte length = (byte)final.length();

		this->buffer.append(&length, sizeof(byte));
		this->buffer.append(final.c_str(), length);
	}

	void writeWordString(const string &value) {
		string final = value.length() > 0xFFFF ? value.substr(0, 0xFFFF) : value;
		word length = (word)final.length();

		this->buffer.append(&length, sizeof(word));
		this->buffer.append(final.c_str(), length);
	}

	void writeDwordString(const string &value) {
		dword length = value.length();

		this->buffer.append(&length, sizeof(dword));
		this->buffer.append(value.c_str(), length);
	}

	void write(const void *ptr, const dword size) {
		this->buffer.append(ptr, size);
	}
};



/**
  * String utilities
  *
  */
string strreplace(const string &value, char src, char dst);
string strreplace(const string &value, const string &src, const string &dst, dword level = 0);
string strtrim(const string &value);
string strstrip(const string &value, const string &separator);
vector<string> strsplit(const string &value, const string &separator, dword limit, bool withseparator = true);

/**
  * Path utilities
  *
  */
vector<string> getdirectoryitems(const string &filename, const vector<string> &exts);
string pathappendslash(const string &filename);
string pathgetdirectory(const string &filename);
dword pathgetsize(const string &filename);
bool pathexists(const string &filename);
bool pathcompare(const string &src1, const string &src2);
bool pathisurl(const string &filename);
bool pathisfile(const string &filename);
bool pathisdirectory(const string &filename);

/**
  * Debug utilities
  *
  */
#ifdef _DEBUG

void debugclear(const char *path);
void debugwrite(const char *text);
void debugwrite(const string &text);

#define DEBUGCLEAR(path)	debugclear(path)
#define DEBUGWRITE(text)	debugwrite(text)

#else

#define DEBUGCLEAR(path)
#define DEBUGWRITE(text)

#endif


#endif //_UTIL_H_INCLUDE_
