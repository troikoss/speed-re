// Original UG2 PC v1.2 0x5474d0, 101 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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
int __cdecl FUN_0075c6c8();
int __stdcall FUN_0053b270(int);
#define iRam00000000 (*(int *)0x00000000)
#define puRam00000004 (*(uint * *)0x00000004)

void __fastcall FUN_005474d0(void *_this,int _edx_unused,int param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  int *piVar3;
  
  if (((param_2 != 0) && (0x4c < param_1)) && (param_1 < 0x5d)) {
    iVar1 = (int)_this + 0x48;
    if (*(int *)((int)_this + 0x48) != iVar1) {
      FUN_0053b270(_this,param_1);
    }
    piVar3 = FUN_0075c6c8(0x10);
    if (piVar3 != (int *)0x0) {
      piVar3[2] = param_2;
      piVar3[3] = param_1;
      puVar2 = *(undefined4 **)((int)_this + 0x4c);
      *puVar2 = piVar3;
      *(int **)((int)_this + 0x4c) = piVar3;
      *piVar3 = iVar1;
      piVar3[1] = (int)puVar2;
      return;
    }
    puRam00000004 = *(undefined4 **)((int)_this + 0x4c);
    *puRam00000004 = 0;
    *(undefined4 *)((int)_this + 0x4c) = 0;
    iRam00000000 = iVar1;
  }
  return;
}


