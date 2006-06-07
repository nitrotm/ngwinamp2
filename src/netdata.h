// netdata.h
#ifndef _NETDATA_H_INCLUDE_
#define _NETDATA_H_INCLUDE_


/**
  * Network request/answer
  *
  */
class NETDATA {
public:
	NETHDR		hdr;
	NGBUFFER	buffer;


	NETDATA(const NETHDR &hdr, const NGBUFFER &data) : buffer(data) {
		memcpy(&this->hdr, &hdr, sizeof(NETHDR));
	}
	NETDATA(dword code, dword param1, dword param2, dword flags, double param3) {
		memset(&this->hdr, 0, sizeof(NETHDR));
		this->hdr.code = code;
		this->hdr.param1 = param1;
		this->hdr.param2 = param2;
		this->hdr.flags = flags;
		this->hdr.param3 = param3;
	}
	NETDATA(dword code, dword param1, dword param2, dword flags, double param3, void *ptr, dword size) {
		memset(&this->hdr, 0, sizeof(NETHDR));
		this->hdr.code = code;
		this->hdr.param1 = param1;
		this->hdr.param2 = param2;
		this->hdr.flags = flags;
		this->hdr.param3 = param3;
		this->hdr.size = size;
		this->buffer.alloc(size);
		this->buffer.write(ptr, 0, size);
	}
	NETDATA(dword code, dword param1, dword param2, dword flags, double param3, const NGBUFFER &data) : buffer(data) {
		memset(&this->hdr, 0, sizeof(NETHDR));
		this->hdr.code = code;
		this->hdr.param1 = param1;
		this->hdr.param2 = param2;
		this->hdr.flags = flags;
		this->hdr.param3 = param3;
		this->hdr.size = data.size();
	}
	virtual ~NETDATA() {
	}


	bool compress(void) {
		if ((this->hdr.flags & NGWINAMP_FILTER_ALLOWZZIP) != 0) {
			if ((this->hdr.flags & NGWINAMP_FILTER_ZZIP) == 0) {
				dword size = this->buffer.size() + 32;
				byte  *tmp = new byte[size];
				int   code;

				this->buffer.read(tmp, 0, this->buffer.size());
				code = zzip_compress(tmp, this->buffer.size(), &size);
				if (code < 0) {
					delete [] tmp;
					return false;
				}
				if (size < this->buffer.size()) {
					this->hdr.flags |= NGWINAMP_FILTER_ZZIP;
					this->hdr.size = size;
					this->hdr.size2 = this->buffer.size();
					this->buffer.set(tmp, size);
				}
				delete [] tmp;
			}
			return true;
		}
		this->hdr.flags &= ~(NGWINAMP_FILTER_ZZIP | NGWINAMP_FILTER_ALLOWZZIP);
		this->hdr.size = this->buffer.size();
		this->hdr.size2 = 0;
		return false;
	}
	bool uncompress(void) {
// note: compatibility
//		if ((this->hdr.flags & NGWINAMP_FILTER_ALLOWZZIP) != 0) {
			if ((this->hdr.flags & NGWINAMP_FILTER_ZZIP) != 0) {
				dword size = this->buffer.size() + 32;
				byte  *tmp = new byte[size];
				int   code;

				this->buffer.read(tmp, 0, this->buffer.size());
				code = zzip_decompress(tmp, this->buffer.size(), &size);
				if (code != 0) {
					delete [] tmp;
					return false;
				}
				this->buffer.set(tmp, size);
				delete [] tmp;

				this->hdr.flags &= ~NGWINAMP_FILTER_ZZIP;
				this->hdr.size = size;
				this->hdr.size2 = 0;
			}
			return true;
//		}
		return false;
	}
};


#endif //_NETDATA_H_INCLUDE_
