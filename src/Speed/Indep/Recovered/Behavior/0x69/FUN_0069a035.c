// Original UG2 PC v1.2 0x69a035, 142 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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



uint __stdcall FUN_0069a035(int param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(*(int *)(param_3 + 0x14) + param_1 * 4);
  iVar2 = *(int *)(*(int *)(param_3 + 0x14) + param_2 * 4);
  if (*(uint *)(iVar2 + 0x38) <= *(uint *)(iVar1 + 0x38)) {
    if (*(uint *)(iVar2 + 0x38) < *(uint *)(iVar1 + 0x38)) {
      return 1;
    }
    if (*(uint *)(iVar2 + 0x3c) <= *(uint *)(iVar1 + 0x3c)) {
      if (*(uint *)(iVar2 + 0x3c) < *(uint *)(iVar1 + 0x3c)) {
        return 1;
      }
      if (*(uint *)(iVar2 + 4) <= *(uint *)(iVar1 + 4)) {
        if (*(uint *)(iVar2 + 4) < *(uint *)(iVar1 + 4)) {
          return 1;
        }
        if (*(uint *)(iVar2 + 0xc) <= *(uint *)(iVar1 + 0xc)) {
          if (*(uint *)(iVar2 + 0xc) < *(uint *)(iVar1 + 0xc)) {
            return 1;
          }
          if (*(uint *)(iVar2 + 8) <= *(uint *)(iVar1 + 8)) {
            if (*(uint *)(iVar2 + 8) < *(uint *)(iVar1 + 8)) {
              return 1;
            }
            if (*(uint *)(iVar2 + 0x10) <= *(uint *)(iVar1 + 0x10)) {
              if (*(uint *)(iVar2 + 0x10) < *(uint *)(iVar1 + 0x10)) {
                return 1;
              }
              if (*(uint *)(iVar2 + 0x14) <= *(uint *)(iVar1 + 0x14)) {
                if (*(uint *)(iVar2 + 0x14) < *(uint *)(iVar1 + 0x14)) {
                  return 1;
                }
                if (*(int *)(iVar2 + 0x18) <= *(int *)(iVar1 + 0x18)) {
                  return (uint)(*(int *)(iVar2 + 0x18) < *(int *)(iVar1 + 0x18));
                }
              }
            }
          }
        }
      }
    }
  }
  return 0xffffffff;
}


