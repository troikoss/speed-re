// Original UG2 PC v1.2 0x60d570, 81 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0060d4f0();


undefined4 FUN_0060d570(void *param_1,int param_2,int param_3)

{
  uint uVar1;
  
  if ((*(void **)(param_2 + 0x280) != param_1) && (*(char *)((int)param_1 + 0x550) != '\0')) {
    uVar1 = FUN_0060d4f0(param_1,param_3);
    if ((uVar1 == 0) &&
       ((*(uint *)((int)param_1 + 0x5b4) & 1 << ((byte)*(undefined4 *)(param_3 + 4) & 0x1f)) != 0))
    {
      return 1;
    }
  }
  return 0;
}


