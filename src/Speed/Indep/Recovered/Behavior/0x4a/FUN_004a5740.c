// Original UG2 PC v1.2 0x4a5740, 346 bytes. BEHAVIOUR-VERIFIED ONLY (20 clean trials, 20 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_00510c30();
#define _DAT_00784548 (*(undefined4 *)0x00784548)
#define DAT_008363f4 (*(undefined4 *)0x008363f4)
#define _DAT_00784540 (*(undefined4 *)0x00784540)
#define _DAT_00787af0 (*(undefined4 *)0x00787af0)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004a5740(void *_this,int _edx_unused,undefined4 param_1,undefined4 param_2,byte param_3)

{
  int iVar1;
  
  if ((*(int *)((int)_this + 4) != 0) && (iVar1 = FUN_00510c30(), iVar1 == 0)) {
    *(undefined4 *)((int)_this + 0x34) = param_1;
    switch(param_2) {
    case 0x42:
      *(float *)((int)_this + 0xc) = (float)(int)-(uint)param_3 * _DAT_00784548;
      return;
    case 0x43:
      *(float *)((int)_this + 0xc) = (float)param_3 * _DAT_00784548;
      return;
    case 0x44:
      *(float *)((int)_this + 0x10) = (float)param_3 * _DAT_00784548;
      return;
    case 0x45:
      *(float *)((int)_this + 0x10) = (float)(int)-(uint)param_3 * _DAT_00784548;
      return;
    case 0x46:
      if (DAT_008363f4 != '\0') {
        *(float *)((int)_this + 0x14) = (float)(int)-(uint)param_3 * _DAT_00784548;
        return;
      }
      break;
    case 0x47:
      if (DAT_008363f4 != '\0') {
        *(float *)((int)_this + 0x14) = (float)param_3 * _DAT_00784548;
        return;
      }
      break;
    case 0x48:
      *(float *)((int)_this + 0xc) = -(((float)param_3 - _DAT_00784540) * _DAT_00787af0);
      return;
    case 0x49:
      *(float *)((int)_this + 0x10) = -(((float)param_3 - _DAT_00784540) * _DAT_00787af0);
      return;
    case 0x4a:
      if (DAT_008363f4 != '\0') {
        *(float *)((int)_this + 0x14) = ((float)param_3 - _DAT_00784540) * _DAT_00787af0;
      }
    }
  }
  return;
}


