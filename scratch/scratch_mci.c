#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "winmm.lib")
int main() {
    mciSendString("open \"https://radio.erb.pw/public/subspace\" alias radio", NULL, 0, NULL);
    mciSendString("play radio", NULL, 0, NULL);
    printf("Playing... Press enter to stop\n");
    getchar();
    mciSendString("stop radio", NULL, 0, NULL);
    mciSendString("close radio", NULL, 0, NULL);
    return 0;
}
