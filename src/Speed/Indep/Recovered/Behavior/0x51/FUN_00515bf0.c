// Original UG2 PC v1.2 0x515bf0, 152 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



undefined4 * __fastcall FUN_00515bf0(void *_this,int _edx_unused,undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar3 = (undefined4 *)((int)param_1 + 0x33);
  if (param_1 + 1 <= puVar3) {
    *param_1 = 0;
    param_1 = param_1 + 1;
  }
  if (param_1 + 1 <= puVar3) {
    *param_1 = 0;
    param_1 = param_1 + 1;
  }
  if ((undefined4 *)((int)param_1 + 1) <= puVar3) {
    *(undefined1 *)param_1 = *(undefined1 *)((int)_this + 0x49);
    param_1 = (undefined4 *)((int)param_1 + 1);
  }
  if ((undefined4 *)((int)param_1 + 1) <= puVar3) {
    *(undefined1 *)param_1 = *(undefined1 *)((int)_this + 0x4a);
    param_1 = (undefined4 *)((int)param_1 + 1);
  }
  if ((undefined4 *)((int)param_1 + 1) <= puVar3) {
    *(undefined1 *)param_1 = *(undefined1 *)((int)_this + 0x4b);
    param_1 = (undefined4 *)((int)param_1 + 1);
  }
  if (param_1 + 5 <= puVar3) {
    puVar2 = (undefined4 *)((int)_this + 0x58);
    puVar4 = param_1;
    for (iVar1 = 5; iVar1 != 0; iVar1 = iVar1 + -1) {
      *puVar4 = *puVar2;
      puVar2 = puVar2 + 1;
      puVar4 = puVar4 + 1;
    }
    param_1 = param_1 + 5;
  }
  if (puVar3 < param_1 + 5) {
    return param_1;
  }
  puVar3 = (undefined4 *)((int)_this + 0x80);
  puVar2 = param_1;
  for (iVar1 = 5; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = *puVar3;
    puVar3 = puVar3 + 1;
    puVar2 = puVar2 + 1;
  }
  return param_1 + 5;
}


