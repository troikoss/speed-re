// Original UG2 PC v1.2 0x685afb, 188 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int a;
typedef int b;
typedef int x;
typedef int y;

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
int __cdecl FUN_0064307c();
#define s_invalid_swizzle___s__007b9f0c ((char *)0x007b9f0c)

uint __fastcall FUN_00685afb(void *_this,int _edx_unused,int param_1)

{
  uint uVar1;
  char cVar2;
  char *pcVar3;
  char *pcVar4;
  int iVar5;
  uint local_c;
  uint local_8;
  
  pcVar3 = *(char **)(param_1 + 8);
  iVar5 = 0;
  local_c = 0;
  if ((pcVar3 == (char *)0x0) || (*pcVar3 == '\0')) {
    local_c = 0xe40000;
  }
  else {
    local_8 = 0x10;
    pcVar4 = pcVar3;
    do {
      cVar2 = *pcVar4;
      if (cVar2 != '\0') {
        if (cVar2 < 'x') {
          if ((cVar2 != 'w') && (cVar2 != 'a')) {
            if (cVar2 == 'b') goto LAB_00685b9d;
            if (cVar2 == 'g') goto LAB_00685ba1;
            if (cVar2 == 'r') goto LAB_00685b4e;
            goto LAB_00685b6a;
          }
          iVar5 = 3;
        }
        else if (cVar2 == 'x') {
LAB_00685b4e:
          iVar5 = 0;
        }
        else if (cVar2 == 'y') {
LAB_00685ba1:
          iVar5 = 1;
        }
        else {
          if (cVar2 != 'z') goto LAB_00685b6a;
LAB_00685b9d:
          iVar5 = 2;
        }
        pcVar4 = pcVar4 + 1;
      }
      uVar1 = local_8 + 2;
      local_c = local_c | iVar5 << ((byte)local_8 & 0x1f);
      local_8 = uVar1;
    } while (uVar1 < 0x18);
    if (*pcVar4 != '\0') {
LAB_00685b6a:
      FUN_0064307c(*(void **)_this,param_1,0x7d4,s_invalid_swizzle___s__007b9f0c,(char)pcVar3);
      *(undefined4 *)((int)_this + 0x4c) = 1;
      local_c = 0;
    }
  }
  return local_c;
}


