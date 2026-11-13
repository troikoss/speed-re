// Original UG2 PC v1.2 0x727f71, 76 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_00721b27();
int __cdecl FUN_0075cfd0();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b4670 (*(undefined4 *)0x008b4670)

size_t FUN_00727f71(void *param_1,int param_2)

{
  size_t _Size;
  size_t sVar1;
  
  sVar1 = 0;
  if (DAT_008b4608 == '\0') {
    return 0xfffffff6;
  }
  _Size = FUN_00721b27(param_2);
  if ((int)_Size < 0) {
    sVar1 = _Size;
  }
  else {
    FUN_0075cfd0(param_1,*(void **)(param_2 * 8 + DAT_008b4670),_Size);
    *(void **)(param_2 * 8 + DAT_008b4670) = param_1;
  }
  return sVar1;
}


