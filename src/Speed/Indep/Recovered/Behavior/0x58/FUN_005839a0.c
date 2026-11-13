// Original UG2 PC v1.2 0x5839a0, 130 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.


typedef unsigned char undefined, undefined1, byte, uchar, bool8;
typedef unsigned short undefined2, ushort, word;
typedef unsigned int undefined4, uint, dword;
typedef unsigned __int64 undefined8, ulonglong;
typedef __int64 longlong;
typedef unsigned char bool;
#define true 1
#define false 0
typedef void (*code)();
typedef double float10;
typedef unsigned short wchar_t16;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;

#define s_MSG_R_BI_CARDESCRIPTION_007a1274 ((char *)0x007a1274)
#define s_MSG_R_BI_DATACRC_007a115c ((char *)0x007a115c)
#define s_MSG_R_BI_DRIVERFINISH_007a11f0 ((char *)0x007a11f0)
#define s_MSG_R_BI_KEEPALIVE_007a11dc ((char *)0x007a11dc)
#define s_MSG_R_CS_CLIENTQUIT_007a11c8 ((char *)0x007a11c8)
#define s_MSG_R_CS_CSTATECHANGE_007a1188 ((char *)0x007a1188)
#define s_MSG_R_SC_CLIENTLEFT_007a11a0 ((char *)0x007a11a0)
#define s_MSG_R_SC_GETAWAYLEADER_007a1144 ((char *)0x007a1144)
#define s_MSG_R_SC_GETAWAYLOST_007a112c ((char *)0x007a112c)
#define s_MSG_R_SC_RESTARTLOAD_007a1114 ((char *)0x007a1114)
#define s_MSG_R_SC_SERVERQUIT_007a11b4 ((char *)0x007a11b4)
#define s_MSG_R_SC_SSTATECHANGE_007a1170 ((char *)0x007a1170)
#define s_MSG_R_SC_WELCOME_007a128c ((char *)0x007a128c)
#define s_MSG_U_BI_CARSPAM_007a1260 ((char *)0x007a1260)
#define s_MSG_U_BI_SCORE_007a1220 ((char *)0x007a1220)
#define s_MSG_U_CS_CLOCKSYNC_007a124c ((char *)0x007a124c)
#define s_MSG_U_SC_CARSPAMCLOCKSYNC_007a1230 ((char *)0x007a1230)
#define s_MSG_U_SC_SYNCANIMATION_007a1208 ((char *)0x007a1208)
#define s_Unknown_0078fe78 ((char *)0x0078fe78)

char * FUN_005839a0(undefined4 param_1)

{
  switch(param_1) {
  case 0:
    return s_MSG_R_SC_WELCOME_007a128c + 4;
  case 1:
    return s_MSG_R_BI_CARDESCRIPTION_007a1274 + 4;
  case 2:
    return s_MSG_U_BI_CARSPAM_007a1260 + 4;
  case 3:
    return s_MSG_U_CS_CLOCKSYNC_007a124c + 4;
  case 4:
    return s_MSG_U_SC_CARSPAMCLOCKSYNC_007a1230 + 4;
  case 5:
    return s_MSG_U_BI_SCORE_007a1220 + 4;
  case 6:
    return s_MSG_U_SC_SYNCANIMATION_007a1208 + 4;
  case 7:
    return s_MSG_R_BI_DRIVERFINISH_007a11f0 + 4;
  case 8:
    return s_MSG_R_BI_KEEPALIVE_007a11dc + 4;
  case 9:
    return s_MSG_R_CS_CLIENTQUIT_007a11c8 + 4;
  case 10:
    return s_MSG_R_SC_SERVERQUIT_007a11b4 + 4;
  case 0xb:
    return s_MSG_R_SC_CLIENTLEFT_007a11a0 + 4;
  case 0xc:
    return s_MSG_R_CS_CSTATECHANGE_007a1188 + 4;
  case 0xd:
    return s_MSG_R_SC_SSTATECHANGE_007a1170 + 4;
  case 0xe:
    return s_MSG_R_BI_DATACRC_007a115c + 4;
  case 0xf:
    return s_MSG_R_SC_GETAWAYLEADER_007a1144 + 4;
  case 0x10:
    return s_MSG_R_SC_GETAWAYLOST_007a112c + 4;
  case 0x11:
    return s_MSG_R_SC_RESTARTLOAD_007a1114 + 4;
  default:
    return s_Unknown_0078fe78;
  }
}


