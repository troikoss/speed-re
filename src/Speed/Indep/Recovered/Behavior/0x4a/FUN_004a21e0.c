// Original UG2 PC v1.2 0x4a21e0, 184 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_005379a0();
int __cdecl FUN_00537c00();
int __cdecl FUN_0055dda0();
#define PTR_DAT_0078fef4 (*(undefined4 *)0x0078fef4)
#define PTR_s_PC_OL_GameRoom_fng_007f44fc ((char *)0x007f44fc)
#define s_FORWARD_0078f5b8 ((char *)0x0078f5b8)
#define s_UNDIM_0079031c ((char *)0x0079031c)

void __fastcall FUN_004a21e0(void *_this, int _edx_unused, uint param_1, int _pad0, int _pad1, int _pad2)

{
  if (param_1 < 0xc98356bb) {
    if (param_1 == 0xc98356ba) {
      if (*(char *)((int)_this + 0x128) != '\0') {
        FUN_005379a0(*(char **)((int)_this + 4));
        *(undefined1 *)((int)_this + 0x128) = 0;
        FUN_0055dda0(9,PTR_s_PC_OL_GameRoom_fng_007f44fc);
        return;
      }
    }
    else {
      if (param_1 == 0x35f8620b) {
        FUN_00537c00(PTR_s_PC_OL_GameRoom_fng_007f44fc,0x47ff4e7c,(byte *)&PTR_DAT_0078fef4,'\x01');
        return;
      }
      if (param_1 == 0x911ab364) {
        FUN_00537c00(*(byte **)((int)_this + 4),0x47ff4e7c,(byte *)s_FORWARD_0078f5b8,'\x01');
        return;
      }
    }
  }
  else if (param_1 == 0xe1fde1d1) {
    FUN_00537c00(PTR_s_PC_OL_GameRoom_fng_007f44fc,0x47ff4e7c,(byte *)s_UNDIM_0079031c,'\x01');
    *(undefined1 *)((int)_this + 0x128) = 1;
  }
  return;
}


