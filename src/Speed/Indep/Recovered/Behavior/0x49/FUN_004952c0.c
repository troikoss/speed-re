// Original UG2 PC v1.2 0x4952c0, 179 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_0043dc00();
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)

char * __fastcall FUN_004952c0(void *_this,int _edx_unused,undefined1 *param_1,undefined1 *param_2,undefined1 *param_3,
            undefined4 param_4,undefined4 param_5,undefined4 param_6,char param_7,char param_8,
            char param_9,undefined1 *param_10)

{
  *(undefined4 *)((int)_this + 0x5c) = param_4;
  *(char *)((int)_this + 0x68) = param_7;
  *(undefined4 *)((int)_this + 0x60) = param_5;
  *(undefined4 *)((int)_this + 100) = param_6;
  *(char *)((int)_this + 0x69) = param_8;
  *(char *)((int)_this + 0x6a) = param_9;
  if (param_1 == (undefined1 *)0x0) {
    param_1 = &DAT_00783fe5;
  }
  FUN_0043dc00(_this,param_1,0x10);
  *(undefined1 *)((int)_this + 0x10) = 0;
  if (param_2 == (undefined1 *)0x0) {
    param_2 = &DAT_00783fe5;
  }
  FUN_0043dc00((char *)((int)_this + 0x14),param_2,0x10);
  *(undefined1 *)((int)_this + 0x24) = 0;
  if (param_3 == (undefined1 *)0x0) {
    param_3 = &DAT_00783fe5;
  }
  FUN_0043dc00((char *)((int)_this + 0x28),param_3,0x32);
  *(undefined1 *)((int)_this + 0x5a) = 0;
  if (param_10 == (undefined1 *)0x0) {
    param_10 = &DAT_00783fe5;
  }
  FUN_0043dc00((char *)((int)_this + 0x6b),param_10,0x32);
  *(undefined1 *)((int)_this + 0x9d) = 0;
  return _this;
}


