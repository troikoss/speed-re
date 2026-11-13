// Original UG2 PC v1.2 0x4a0b80, 139 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_00850088 (*(undefined4 *)0x00850088)

int __stdcall FUN_004a0b80(int param_1)

{
  int iVar1;
  int iVar2;
  int local_1c [7];
  
  local_1c[0] = 0;
  local_1c[1] = 1;
  local_1c[2] = 2;
  local_1c[3] = 3;
  local_1c[4] = 10;
  local_1c[5] = 4;
  local_1c[6] = 8;
  iVar1 = 0;
  do {
    if (local_1c[iVar1] == DAT_00850088) {
      if (iVar1 < 7) goto LAB_004a0bd8;
      break;
    }
    iVar1 = iVar1 + 1;
  } while (iVar1 < 7);
  iVar1 = 0;
LAB_004a0bd8:
  iVar2 = (iVar1 + param_1) % 7;
  if (iVar2 < 0) {
    DAT_00850088 = local_1c[iVar2 + 7];
    return (iVar1 + param_1) / 7;
  }
  DAT_00850088 = local_1c[iVar2];
  return local_1c[iVar2];
}


