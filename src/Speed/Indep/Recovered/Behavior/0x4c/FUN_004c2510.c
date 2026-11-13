// Original UG2 PC v1.2 0x4c2510, 68 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0050b8c0();
int __cdecl FUN_0050cea0();
int __cdecl FUN_0052cf60();
int __cdecl FUN_005379a0();
int __stdcall FUN_005f4240(int);
#define DAT_008900d8 (*(undefined4 *)0x008900d8)

void __stdcall FUN_004c2510(byte *param_1, int param_2, int _pad0, int _pad1, int _pad2)

{
  char *pcVar1;
  int iVar2;
  
  if (param_2 == 0xc407210) {
    FUN_005f4240(DAT_008900d8,2);
    FUN_0050b8c0(param_1,(undefined *)0x0);
    pcVar1 = FUN_0050cea0();
    iVar2 = FUN_0052cf60(pcVar1);
    if ((char)iVar2 != '\0') {
      pcVar1 = FUN_0050cea0();
      FUN_005379a0(pcVar1);
    }
  }
  return;
}


