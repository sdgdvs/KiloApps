import sys
import re

with open("c:/KiloApps/KiloApps/KBBS/main.c", "r") as f:
    code = f.read()

# 1. Update variables
code = code.replace(
    "char transferStatusMsg[128] = \"\";\n\nHWND hBtnXmDl, hBtnXmUl;",
    """char transferStatusMsg[128] = "";

HWND hBtnXmDl, hBtnXmUl, hBtnZmDl, hBtnZmUl;
unsigned char sigBuf[6] = {0};

/* ZMODEM State */
#define ZM_STATE_IDLE 0
#define ZM_STATE_HEX_HEADER 1
#define ZM_STATE_BIN_HEADER 2
#define ZM_STATE_DATA_CRC 3
#define ZM_STATE_DATA_READ 4
#define ZM_STATE_WAIT_ZRINIT 5
#define ZM_STATE_WAIT_ZRPOS 6
#define ZM_STATE_WAIT_FIN_ACK 7
#define ZM_STATE_WAIT_ACK 8
#define ZM_STATE_SENDING_DATA 9

int zmState = ZM_STATE_IDLE;
int zmExpectedData = 0; /* 1=FILE, 2=DATA */
unsigned char* zmDlBuf = NULL;
int zmDlBufLen = 0;
int zmDlEscaped = 0;
char zmDlHdrBuf[16];
int zmDlHdrLen = 0;
int zmDlBin32 = 0;
int zmDlDataType = 0;
unsigned char zmDlCrcBuf[4];
int zmDlCrcLen = 0;

int zmUlOffset = 0;
int zmFileSize = 0;
char zmUlName[260] = "";

void StartTransfer(int type);
void HandleZmodemHeader(unsigned char type, unsigned char f0, unsigned char f1, unsigned char f2, unsigned char f3);
void SendZmodemUlChunks(void);"""
)

# 2. Add ZMODEM helper functions above ProcessTransferByte
zmodem_funcs = """
int HexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}
unsigned char ParseHex2(char c1, char c2) { return (unsigned char)((HexVal(c1) << 4) | HexVal(c2)); }

unsigned short ZmodemUpdateCrc(unsigned short crc, unsigned char c) {
    int i;
    crc ^= (c << 8);
    for (i = 0; i < 8; i++) {
        if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
        else crc = crc << 1;
    }
    return crc & 0xFFFF;
}

void WsSendEscaped(unsigned char* arr, int len) {
    unsigned char out[4096];
    int p = 0, i;
    for (i=0; i<len && p<4090; i++) {
        out[p++] = arr[i];
        if (arr[i] == 0xFF) out[p++] = 0xFF; /* telnet escape */
    }
    send(sock, (char*)out, p, 0);
    bytesTx += p;
}

void ZmodemSendHex(unsigned char type, unsigned int flags) {
    unsigned char f0 = flags & 0xff, f1 = (flags >> 8) & 0xff, f2 = (flags >> 16) & 0xff, f3 = (flags >> 24) & 0xff;
    unsigned short crc = 0;
    char hexStr[32];
    crc = ZmodemUpdateCrc(crc, type); crc = ZmodemUpdateCrc(crc, f0); crc = ZmodemUpdateCrc(crc, f1);
    crc = ZmodemUpdateCrc(crc, f2); crc = ZmodemUpdateCrc(crc, f3); crc = ZmodemUpdateCrc(crc, 0); crc = ZmodemUpdateCrc(crc, 0);
    wsprintfA(hexStr, "**\\x18\\x42%02x%02x%02x%02x%02x%02x%02x\\r\\n\\x11", type, f0, f1, f2, f3, (crc>>8)&0xff, crc&0xff);
    WsSendEscaped((unsigned char*)hexStr, my_strlen(hexStr));
}

int ZmodemEscape(unsigned char* inBuf, int inLen, unsigned char* outBuf) {
    int p = 0, i;
    for(i=0; i<inLen; i++) {
        unsigned char c = inBuf[i];
        if (c == 0x18 || c == 0x10 || c == 0x11 || c == 0x13 || c == 0x0D || c == 0x8D || c == 0x90 || c == 0x91 || c == 0x93) {
            outBuf[p++] = 0x18;
            if (c == 0x0D) outBuf[p++] = 0x4D; else if (c == 0x8D) outBuf[p++] = 0xCD; else outBuf[p++] = c ^ 0x40;
        } else outBuf[p++] = c;
    }
    return p;
}

void ZmodemMakeBinHeader(unsigned char type, unsigned int flags, unsigned char* out, int* outLen) {
    unsigned char f0 = flags & 0xff, f1 = (flags >> 8) & 0xff, f2 = (flags >> 16) & 0xff, f3 = (flags >> 24) & 0xff;
    unsigned short crc = 0;
    unsigned char unesc[7];
    crc = ZmodemUpdateCrc(crc, type); crc = ZmodemUpdateCrc(crc, f0); crc = ZmodemUpdateCrc(crc, f1);
    crc = ZmodemUpdateCrc(crc, f2); crc = ZmodemUpdateCrc(crc, f3); crc = ZmodemUpdateCrc(crc, 0); crc = ZmodemUpdateCrc(crc, 0);
    unesc[0]=type; unesc[1]=f0; unesc[2]=f1; unesc[3]=f2; unesc[4]=f3; unesc[5]=(unsigned char)((crc>>8)&0xff); unesc[6]=(unsigned char)(crc&0xff);
    out[0] = 0x2A; out[1] = 0x18; out[2] = 0x41;
    *outLen = 3;
    *outLen += ZmodemEscape(unesc, 7, out + 3);
}

void ZmodemSendDataPacket(unsigned char* hdrBytes, int hdrLen, unsigned char* data, int dataLen, unsigned char endType) {
    unsigned char escData[2048];
    int escLen = ZmodemEscape(data, dataLen, escData);
    unsigned short crc = 0;
    int i, p = 0;
    unsigned char unescCrc[2];
    unsigned char escCrc[4];
    int escCrcLen;
    unsigned char out[4096];
    
    for(i=0; i<dataLen; i++) crc = ZmodemUpdateCrc(crc, data[i]);
    crc = ZmodemUpdateCrc(crc, endType); crc = ZmodemUpdateCrc(crc, 0); crc = ZmodemUpdateCrc(crc, 0);
    
    unescCrc[0] = (unsigned char)((crc>>8)&0xff); unescCrc[1] = (unsigned char)(crc&0xff);
    escCrcLen = ZmodemEscape(unescCrc, 2, escCrc);
    
    for(i=0; i<hdrLen; i++) out[p++] = hdrBytes[i];
    for(i=0; i<escLen; i++) out[p++] = escData[i];
    out[p++] = 0x18; out[p++] = endType;
    for(i=0; i<escCrcLen; i++) out[p++] = escCrc[i];
    WsSendEscaped(out, p);
}

void StartTransfer(int type) {
    if (sock == INVALID_SOCKET) return;
    if (transferActive) return;
    transferActive = type;
    transferState = 0;
    transferBlockNum = 1;
    transferBytesTotal = 0;
    if (type == 1) {
        unsigned char nak = XMODEM_NAK;
        wsprintfA(transferStatusMsg, "Waiting for host...");
        send(sock, (char*)&nak, 1, 0);
    } else if (type == 3) {
        zmState = ZM_STATE_IDLE; zmExpectedData = 0;
        wsprintfA(transferStatusMsg, "Starting ZMODEM DL...");
        ZmodemSendHex(1, 0); /* ZRINIT */
        if (zmDlBuf == NULL) zmDlBuf = (unsigned char*)GlobalAlloc(GPTR, 4096);
    } else if (type == 4) {
        zmState = ZM_STATE_WAIT_ZRINIT;
        zmUlOffset = 0;
        wsprintfA(transferStatusMsg, "Sending ZRQINIT...");
        ZmodemSendHex(0, 0); /* ZRQINIT */
    }
    InvalidateRect(hMain, NULL, FALSE);
}

void HandleZmodemHeader(unsigned char type, unsigned char f0, unsigned char f1, unsigned char f2, unsigned char f3) {
    if (transferActive == 3) {
        if (type == 0) { ZmodemSendHex(1, 0); } /* ZRINIT */
        else if (type == 4) { zmExpectedData = 1; zmState = ZM_STATE_DATA_READ; zmDlBufLen = 0; } /* ZFILE */
        else if (type == 10) { zmExpectedData = 2; zmState = ZM_STATE_DATA_READ; zmDlBufLen = 0; } /* ZDATA */
        else if (type == 11) { ZmodemSendHex(1, 0); } /* ZEOF */
        else if (type == 8) { /* ZFIN */
            unsigned char ooo[2] = {0x4F, 0x4F};
            ZmodemSendHex(8, 0);
            WsSendEscaped(ooo, 2);
            EndTransfer("Download Complete");
        }
    } else if (transferActive == 4) {
        unsigned int offset = f0 | (f1<<8) | (f2<<16) | (f3<<24);
        if (type == 1) { /* ZRINIT */
            if (zmState == ZM_STATE_WAIT_ZRINIT) {
                unsigned char hdr[16]; int hdrLen;
                unsigned char payload[512];
                int pLen = 0, sLen = 0;
                char sizeStr[32];
                zmState = ZM_STATE_WAIT_ZRPOS;
                ZmodemMakeBinHeader(4, 0, hdr, &hdrLen);
                while(zmUlName[pLen]) { payload[pLen] = zmUlName[pLen]; pLen++; }
                payload[pLen++] = 0;
                my_itoa(zmFileSize, sizeStr);
                while(sizeStr[sLen]) { payload[pLen++] = sizeStr[sLen++]; }
                payload[pLen++] = 0;
                ZmodemSendDataPacket(hdr, hdrLen, payload, pLen, 0x6B); /* ZCRCW */
            } else if (zmState == ZM_STATE_WAIT_FIN_ACK) {
                zmState = ZM_STATE_IDLE; EndTransfer("Upload Complete");
            }
        } else if (type == 9) { /* ZRPOS */
            if (zmState == ZM_STATE_WAIT_ZRPOS || zmState == ZM_STATE_SENDING_DATA) {
                unsigned char hdr[16]; int hdrLen;
                zmUlOffset = offset; zmState = ZM_STATE_SENDING_DATA;
                SetFilePointer(hTransferFile, zmUlOffset, NULL, FILE_BEGIN);
                ZmodemMakeBinHeader(10, zmUlOffset, hdr, &hdrLen);
                WsSendEscaped(hdr, hdrLen);
                SendZmodemUlChunks();
            }
        } else if (type == 3) { /* ZACK */
            if (zmState == ZM_STATE_WAIT_ACK) { zmState = ZM_STATE_SENDING_DATA; SendZmodemUlChunks(); }
        } else if (type == 8) { /* ZFIN */
            unsigned char ooo[2] = {0x4F, 0x4F};
            WsSendEscaped(ooo, 2);
            EndTransfer("Upload Complete");
        }
    }
}

void SendZmodemUlChunks(void) {
    unsigned char chunk[1024];
    DWORD readBytes = 0;
    unsigned char endType;
    if (transferActive != 4) return;
    if (zmUlOffset >= zmFileSize) {
        ZmodemSendHex(11, zmUlOffset); /* ZEOF */
        zmState = ZM_STATE_WAIT_ZRINIT;
        return;
    }
    ReadFile(hTransferFile, chunk, 1024, &readBytes, NULL);
    zmUlOffset += readBytes;
    endType = (zmUlOffset >= zmFileSize) ? 0x6B : 0x69; /* ZCRCW : ZCRCG */
    if (endType == 0x6B) zmState = ZM_STATE_WAIT_ACK;
    ZmodemSendDataPacket(NULL, 0, chunk, readBytes, endType);
    transferBytesTotal = zmUlOffset;
    wsprintfA(transferStatusMsg, "Uploading... %d / %d bytes", zmUlOffset, zmFileSize);
    InvalidateRect(hMain, NULL, FALSE);
    if (endType == 0x69) {
        PostMessageA(hMain, WM_USER + 2, 0, 0);
    }
}

void ProcessTransferByte(unsigned char ch) {
"""
code = code.replace("void ProcessTransferByte(unsigned char ch) {\n", zmodem_funcs)

# 3. Add ZMODEM handling to ProcessTransferByte
zmodem_proc = """    if (transferActive == 3 || transferActive == 4) {
        if (ch == 0x11 || ch == 0x13 || ch == 0x91 || ch == 0x93) return; /* ignore XON/XOFF */
        if (ch == 0x18) { zmDlEscaped = 1; return; }
        if (zmDlEscaped) {
            zmDlEscaped = 0;
            if (ch == 0x4D || ch == 0xcd) ch = 0x0D;
            else if (ch == 0x8d) ch = 0x8D;
            else if (ch == 0x42) { zmState = ZM_STATE_HEX_HEADER; zmDlHdrLen = 0; return; } /* B */
            else if (ch == 0x41 || ch == 0x43) { zmState = ZM_STATE_BIN_HEADER; zmDlHdrLen = 0; zmDlBin32 = (ch==0x43); return; } /* A/C */
            else if (ch >= 0x68 && ch <= 0x6B) { zmDlDataType = ch; zmState = ZM_STATE_DATA_CRC; zmDlCrcLen = 0; return; }
            else ch = ch ^ 0x40; /* unescape */
        }
        
        if (zmState == ZM_STATE_HEX_HEADER) {
            zmDlHdrBuf[zmDlHdrLen++] = (char)ch;
            if (zmDlHdrLen == 14) {
                unsigned char type = ParseHex2(zmDlHdrBuf[0], zmDlHdrBuf[1]);
                unsigned char f0 = ParseHex2(zmDlHdrBuf[2], zmDlHdrBuf[3]);
                unsigned char f1 = ParseHex2(zmDlHdrBuf[4], zmDlHdrBuf[5]);
                unsigned char f2 = ParseHex2(zmDlHdrBuf[6], zmDlHdrBuf[7]);
                unsigned char f3 = ParseHex2(zmDlHdrBuf[8], zmDlHdrBuf[9]);
                zmState = ZM_STATE_IDLE; HandleZmodemHeader(type, f0, f1, f2, f3);
            }
        } else if (zmState == ZM_STATE_BIN_HEADER) {
            zmDlHdrBuf[zmDlHdrLen++] = (char)ch;
            if (zmDlHdrLen == (zmDlBin32 ? 9 : 7)) {
                unsigned char type = zmDlHdrBuf[0], f0 = zmDlHdrBuf[1], f1 = zmDlHdrBuf[2], f2 = zmDlHdrBuf[3], f3 = zmDlHdrBuf[4];
                zmState = ZM_STATE_IDLE; HandleZmodemHeader(type, f0, f1, f2, f3);
            }
        } else if (zmState == ZM_STATE_DATA_CRC) {
            zmDlCrcBuf[zmDlCrcLen++] = ch;
            if (zmDlCrcLen == (zmDlBin32 ? 4 : 2)) {
                if (zmExpectedData == 1) { /* FILE */
                    zmExpectedData = 0; zmState = ZM_STATE_IDLE; zmDlBufLen = 0;
                    ZmodemSendHex(9, 0); /* ZRPOS 0 */
                } else if (zmExpectedData == 2) { /* DATA */
                    DWORD written;
                    WriteFile(hTransferFile, zmDlBuf, zmDlBufLen, &written, NULL);
                    transferBytesTotal += zmDlBufLen;
                    wsprintfA(transferStatusMsg, "Downloading... %d bytes", transferBytesTotal);
                    InvalidateRect(hMain, NULL, FALSE);
                    zmDlBufLen = 0;
                    if (zmDlDataType == 0x6B) ZmodemSendHex(3, transferBytesTotal); /* ZACK */
                    zmState = ZM_STATE_IDLE;
                } else {
                    zmState = ZM_STATE_IDLE;
                }
            }
        } else if (zmState == ZM_STATE_DATA_READ) {
            if (zmDlBufLen < 4096) {
                zmDlBuf[zmDlBufLen++] = ch;
            }
        }
        return;
    }
"""
code = code.replace("    if (transferActive == 1) { /* Xmodem DL */\n", zmodem_proc + "    if (transferActive == 1) { /* Xmodem DL */\n")

# 4. Modify ProcessTelnetByte to auto-detect ZRQINIT
telnet_detect = """        case TEL_STATE_NORMAL:
            if (ch == TELNET_IAC) {
                telState = TEL_STATE_IAC;
            } else {
                sigBuf[0]=sigBuf[1]; sigBuf[1]=sigBuf[2]; sigBuf[2]=sigBuf[3]; sigBuf[3]=sigBuf[4]; sigBuf[4]=sigBuf[5]; sigBuf[5]=ch;
                if (sigBuf[0]==0x2A && sigBuf[1]==0x2A && sigBuf[2]==0x18 && sigBuf[3]==0x42 && sigBuf[4]==0x30 && sigBuf[5]==0x30) {
                    if (!transferActive) {
                        OPENFILENAMEA ofn = {0}; char szFile[260] = {0};
                        ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hMain; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                        if (GetSaveFileNameA(&ofn)) {
                            hTransferFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                            if (hTransferFile != INVALID_HANDLE_VALUE) {
                                StartTransfer(3);
                            }
                        }
                        return;
                    }
                }
                if (transferActive) ProcessTransferByte(ch);
                else ProcessByte(ch);
            }
            break;"""
code = code.replace("""        case TEL_STATE_NORMAL:
            if (ch == TELNET_IAC) {
                telState = TEL_STATE_IAC;
            } else {
                if (transferActive) ProcessTransferByte(ch);
                else ProcessByte(ch);
            }
            break;""", telnet_detect)

# 5. Expand window size, add buttons
code = code.replace("winW = dpiScale(870);", "winW = dpiScale(920);")
ui_btns = """            hBtnXmDl = CreateWindowA("BUTTON", "DL (XM)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(566), dpiScale(4), dpiScale(56), dpiScale(22), hwnd, (HMENU)103, 0, 0);

            hBtnXmUl = CreateWindowA("BUTTON", "UL (XM)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(626), dpiScale(4), dpiScale(56), dpiScale(22), hwnd, (HMENU)104, 0, 0);
                
            hBtnZmDl = CreateWindowA("BUTTON", "DL (ZM)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(686), dpiScale(4), dpiScale(56), dpiScale(22), hwnd, (HMENU)105, 0, 0);

            hBtnZmUl = CreateWindowA("BUTTON", "UL (ZM)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(746), dpiScale(4), dpiScale(56), dpiScale(22), hwnd, (HMENU)106, 0, 0);

            hEcho = CreateWindowA("BUTTON", "Echo", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                dpiScale(806), dpiScale(4), dpiScale(52), dpiScale(22), hwnd, (HMENU)102, 0, 0);

            hBtnSettings = CreateWindowA("BUTTON", "Set", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                dpiScale(862), dpiScale(4), dpiScale(36), dpiScale(22), hwnd, (HMENU)110, 0, 0);"""
                
code = re.sub(r'hBtnXmDl = CreateWindowA.*?\(HMENU\)110, 0, 0\);', ui_btns, code, flags=re.DOTALL)
code = code.replace("SendMessageA(hBtnXmUl, WM_SETFONT, (WPARAM)hFont, TRUE);", "SendMessageA(hBtnXmUl, WM_SETFONT, (WPARAM)hFont, TRUE);\n            SendMessageA(hBtnZmDl, WM_SETFONT, (WPARAM)hFont, TRUE);\n            SendMessageA(hBtnZmUl, WM_SETFONT, (WPARAM)hFont, TRUE);")

# 6. Button events for 105 and 106, and WM_USER+2
code = re.sub(r'ofn\.Flags = OFN_PATHMUSTEXIST \| OFN_FILEMUSTEXIST;.*?InvalidateRect\(hwnd, NULL, FALSE\);\s*\}\s*\}\s*\}\s*\}\s*\} else if \(LOWORD\(wParam\) == 110\) \{',
    r'''ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if (GetOpenFileNameA(&ofn)) {
                        hTransferFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hTransferFile != INVALID_HANDLE_VALUE) {
                            transferActive = 2;
                            transferState = 0;
                            transferBlockNum = 1;
                            transferBytesTotal = 0;
                            wsprintfA(transferStatusMsg, "Waiting for NAK...");
                            InvalidateRect(hwnd, NULL, FALSE);
                        }
                    }
                }
            } else if (LOWORD(wParam) == 105) {
                if (sock != INVALID_SOCKET && !transferActive) {
                    OPENFILENAMEA ofn = {0}; char szFile[260] = {0};
                    ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "All Files\\0*.*\\0";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
                    if (GetSaveFileNameA(&ofn)) {
                        hTransferFile = CreateFileA(ofn.lpstrFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hTransferFile != INVALID_HANDLE_VALUE) { StartTransfer(3); }
                    }
                }
            } else if (LOWORD(wParam) == 106) {
                if (sock != INVALID_SOCKET && !transferActive) {
                    OPENFILENAMEA ofn = {0}; char szFile[260] = {0};
                    ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = hwnd; ofn.lpstrFile = szFile; ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = "All Files\\0*.*\\0";
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                    if (GetOpenFileNameA(&ofn)) {
                        hTransferFile = CreateFileA(ofn.lpstrFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                        if (hTransferFile != INVALID_HANDLE_VALUE) {
                            int i = my_strlen(szFile) - 1;
                            while(i>=0 && szFile[i]!='\\\\' && szFile[i]!='/') i--;
                            my_strcpy(zmUlName, szFile + i + 1);
                            zmFileSize = GetFileSize(hTransferFile, NULL);
                            StartTransfer(4);
                        }
                    }
                }
            } else if (LOWORD(wParam) == 110) {''', code, flags=re.DOTALL)

wm_user = """        case (WM_USER + 2): {
            SendZmodemUlChunks();
            return 0;
        }
        case WM_SOCKET: {"""
code = code.replace("        case WM_SOCKET: {", wm_user)

overlay = """DrawTextA(winDC, transferActive == 1 ? "XMODEM DOWNLOAD" : "XMODEM UPLOAD", -1, &rTitle, DT_CENTER | DT_TOP);"""
code = code.replace(overlay, """
                char* tStr = "TRANSFER";
                if(transferActive==1) tStr="XMODEM DOWNLOAD";
                else if(transferActive==2) tStr="XMODEM UPLOAD";
                else if(transferActive==3) tStr="ZMODEM DOWNLOAD";
                else if(transferActive==4) tStr="ZMODEM UPLOAD";
                DrawTextA(winDC, tStr, -1, &rTitle, DT_CENTER | DT_TOP);""")

with open("c:/KiloApps/KiloApps/KBBS/main.c", "w") as f:
    f.write(code)
