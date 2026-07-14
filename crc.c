#include <stdio.h>  
unsigned short updcrc(unsigned short crc, unsigned char c) { crc ^= c << 8; for(int i=0; i<8; i++) if (crc & 0x8000) crc = (crc << 1) ^ 0x1021; else crc = crc << 1; return crc; }  
int main() { unsigned short crc = 0; crc = updcrc(crc, 1); crc = updcrc(crc, 0); crc = updcrc(crc, 0); crc = updcrc(crc, 0); crc = updcrc(crc, 0); crc = updcrc(crc, 0); crc = updcrc(crc, 0); printf(\" "%%04x\n\, crc); return 0; }  
