
#ifndef _SAMPCIPHER_H
#define _SAMPCIPHER_H

#ifdef SAMPSRV
bool DecryptData(char* dest, char* src, int* len);
#else
void EncryptData(char* dest, char* src, int* len);
#endif
void SetXorKey(int key);

#endif
