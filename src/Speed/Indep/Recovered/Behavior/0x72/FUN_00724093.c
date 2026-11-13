// Original UG2 PC v1.2 0x724093, 151 bytes. BEHAVIOUR-VERIFIED ONLY (38 clean trials, 2 both-fault):
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
int __cdecl FUN_00724fc5();
int __cdecl FUN_00725480();
int __cdecl FUN_00727fbd();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)
#define DAT_008b466c (*(undefined4 *)0x008b466c)
#define DAT_008b46ec (*(undefined4 *)0x008b46ec)

int FUN_00724093(uint param_1,short param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  int local_8;
  
  if (DAT_008b4608 == '\0') {
    return -10;
  }
  iVar1 = -0x4000;
  if ((param_3 < -0x4000) || (iVar1 = 0x3fff, 0x3fff < param_3)) {
    param_3 = iVar1;
  }
  uVar2 = FUN_00725480(param_1);
  if (-1 < (int)uVar2) {
    local_8 = -1;
    while( true ) {
      iVar1 = FUN_00727fbd(uVar2,&local_8);
      if (iVar1 == 0) break;
      iVar1 = local_8 * 0x90 + DAT_008b466c;
      if (*(char *)(iVar1 + 0x23) == '\x01') {
        *(short *)(iVar1 + 0x1e) = (short)param_3;
        *(short *)(iVar1 + 0x1c) = *(short *)(iVar1 + 0x50) + *(short *)(iVar1 + 0x4c) + param_2;
        if (DAT_008b46ec == 0) {
          *(undefined2 *)(iVar1 + 0x1c) = 0;
        }
        FUN_00724fc5(local_8);
      }
    }
  }
  return uVar2;
}


