// netaddr.h
#ifndef _NETADDR_H_INCLUDE_
#define _NETADDR_H_INCLUDE_


// adresse réseau
class NETADDR {
public:
	byte ip[4];
	byte mask[4];

	NETADDR() {
		this->ip[0] = this->ip[1] = this->ip[2] = this->ip[3] = 0;
		this->mask[0] = this->mask[1] = this->mask[2] = this->mask[3] = 0x0;
	}
	NETADDR(byte ip1, byte ip2, byte ip3, byte ip4) {
		this->ip[0] = ip1;
		this->ip[1] = ip2;
		this->ip[2] = ip3;
		this->ip[3] = ip4;
		this->mask[0] = this->mask[1] = this->mask[2] = this->mask[3] = 0xff;
	}
	NETADDR(byte ip1, byte ip2, byte ip3, byte ip4, byte mask1, byte mask2, byte mask3, byte mask4) {
		this->ip[0] = ip1;
		this->ip[1] = ip2;
		this->ip[2] = ip3;
		this->ip[3] = ip4;
		this->mask[0] = mask1;
		this->mask[1] = mask2;
		this->mask[2] = mask3;
		this->mask[3] = mask4;
	}
	NETADDR(byte ip[4]) {
		this->ip[0] = ip[0];
		this->ip[1] = ip[1];
		this->ip[2] = ip[2];
		this->ip[3] = ip[3];
		this->mask[0] = this->mask[1] = this->mask[2] = this->mask[3] = 0xff;
	}
	NETADDR(byte ip[4], byte mask[4]) {
		this->ip[0] = ip[0];
		this->ip[1] = ip[1];
		this->ip[2] = ip[2];
		this->ip[3] = ip[3];
		this->mask[0] = mask[0];
		this->mask[1] = mask[1];
		this->mask[2] = mask[2];
		this->mask[3] = mask[3];
	}
	NETADDR(const NETADDR &src) {
		this->ip[0] = src.ip[0];
		this->ip[1] = src.ip[1];
		this->ip[2] = src.ip[2];
		this->ip[3] = src.ip[3];
		this->mask[0] = src.mask[0];
		this->mask[1] = src.mask[1];
		this->mask[2] = src.mask[2];
		this->mask[3] = src.mask[3];
	}

	static vector<NETADDR> parse(const string &value) {
		vector<NETADDR> items;
		vector<string>	values = strsplit(value, " ", 0);

		for (dword i = 0; i < values.size(); i++) {
			string value = strtrim(values[i]);

			if (value.length() > 0) {
				vector<string> item = strsplit(value, "/", 0);

				if (item.size() == 1) {
					vector<string> ip = strsplit(item[0], ".", 7);

					if (ip.size() == 7) {
						items.push_back(NETADDR(atoi(ip[0].c_str()), atoi(ip[2].c_str()), atoi(ip[4].c_str()), atoi(ip[6].c_str())));
					}
				} else if (item.size() == 3) {
					vector<string> ip = strsplit(item[0], ".", 7);

					if (ip.size() == 7) {
						vector<string> mask = strsplit(item[2], ".", 7);

						if (mask.size() == 1) {
							dword masku = atoi(mask[0].c_str());
							dword maskt = 0;

							for (dword i = 0; i < masku; i++) {
								maskt |= 1 << i;
							}
							items.push_back(NETADDR(atoi(ip[0].c_str()), atoi(ip[2].c_str()), atoi(ip[4].c_str()), atoi(ip[6].c_str()), 
								(byte)(maskt >> 0) & 0xFF, (byte)(maskt >> 8) & 0xFF, (byte)(maskt >> 16) & 0xFF, (byte)(maskt >> 24) & 0xFF));
						} else if (mask.size() == 7) {
							items.push_back(NETADDR(atoi(ip[0].c_str()), atoi(ip[2].c_str()), atoi(ip[4].c_str()), atoi(ip[6].c_str()),
								atoi(mask[0].c_str()), atoi(mask[2].c_str()), atoi(mask[4].c_str()), atoi(mask[6].c_str())));
						}
					}
				}
			}
		}
		return items;
	}
};


#endif //_NETADDR_H_INCLUDE_
