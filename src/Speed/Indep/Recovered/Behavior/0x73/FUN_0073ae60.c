// Original UG2 PC v1.2 0x73ae60, 49 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0045a3d0();


int FUN_0073ae60(int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  
  iVar1 = FUN_0045a3d0(param_1,param_2);
  uVar2 = iVar1 + (param_1 + 0x11) * 0x10;
  uVar3 = uVar2 & 0x8000000f;
  if ((int)uVar3 < 0) {
    uVar3 = (uVar3 - 1 | 0xfffffff0) + 1;
  }
  return (uVar2 - uVar3) + 0x10;
}


