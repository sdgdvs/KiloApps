#include <windows.h>
__declspec(dllimport) int __cdecl sprintf(char*, const char*, ...);
__declspec(dllimport) double __cdecl atof(const char*);
__declspec(dllimport) double __cdecl sin(double);

void __stdcall MainEntry() {
    char buf[64];
    double v = atof("1.57079632679");
    v = sin(v);
    sprintf(buf, "%f", v);
    MessageBoxA(0, buf, "Result", 0);
    ExitProcess(0);
}
