// Original UG2 PC v1.2 0x575470, 57 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined8 __stdcall FUN_00575470(int param_1, uint param_2, uint param_3, int _pad0, int _pad1, int _pad2)

{
  if ((param_3 <= *(uint *)(param_1 + 0x94)) &&
     ((param_3 < *(uint *)(param_1 + 0x94) || (param_2 <= *(uint *)(param_1 + 0x90))))) {
    *(uint *)(param_1 + 0x98) = param_2;
    *(uint *)(param_1 + 0x9c) = param_3;
  }
  return *(undefined8 *)(param_1 + 0x98);
}


