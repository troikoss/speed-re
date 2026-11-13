// Original UG2 PC v1.2 0x53a270, 64 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_00537c60();
#define DAT_00838998 (*(undefined4 *)0x00838998)
#define DAT_00838990 (*(undefined4 *)0x00838990)

void __fastcall FUN_0053a270(void *_this, int _edx_unused, byte *param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  if (param_2 == 0xc407210) {
    if (*(int *)((int)_this + 0x48) == 3) {
      DAT_00838998 = 0;
    }
    DAT_00838990 = *(undefined4 *)((int)_this + 0x48);
    FUN_00537c60(param_1,0x47ff4e7c,-0x219100cc,'\x01');
  }
  return;
}


