// Original UG2 PC v1.2 0x5d6040, 57 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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



void __fastcall FUN_005d6040(void *_this,int _edx_unused,int param_1,undefined4 *param_2,undefined4 *param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  puVar1 = (undefined4 *)(param_1 * 0x5c + 0xb0 + (int)_this);
  *puVar1 = 0xffffffff;
  uVar2 = param_2[1];
  puVar1[1] = *param_2;
  puVar1[2] = uVar2;
  uVar2 = param_3[1];
  puVar1[3] = *param_3;
  puVar1[4] = uVar2;
  *(undefined1 *)(puVar1 + 5) = 1;
  return;
}


