#include <windows.h>
void __stdcall MainEntry() {
    double d = 1.5;
    int i = (int)d;
    ExitProcess(i);
}
