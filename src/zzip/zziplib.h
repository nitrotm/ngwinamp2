// zziplib.h : librairie de compression
#ifndef ZZIPLIB_H_INCLUDE
#define ZZIPLIB_H_INCLUDE

extern "C"
{
// Compresse / décompresse des données
int zzip_compress(unsigned char* data, unsigned long in_size, unsigned long* out_size);
int zzip_decompress(unsigned char* data, unsigned long in_size, unsigned long* out_size);
}

#endif //ZZIPLIB_H_INCLUDE
