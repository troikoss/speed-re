// Original UG2 PC v1.2 0x468380, 95 bytes. BEHAVIOUR-VERIFIED ONLY (22 clean trials, 18 both-fault):
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
int __cdecl FUN_00468050();
int __cdecl FUN_00468160();


void FUN_00468380(uint param_1,undefined4 param_2,uint *param_3,int param_4)

{
  uint uVar1;
  uint uVar2;
  
  uVar1 = FUN_00468160(param_1,*param_3,'\0');
  uVar2 = 0x7fff;
  if (param_4 != 0) {
    uVar2 = FUN_00468160(param_1,param_3[1],'\0');
  }
  FUN_00468050(((int)((param_4 * -2 + 0x7fff) * uVar1) >> 0xf) + ((int)(param_4 * 2 * uVar2) >> 0xf)
              );
  return;
}


