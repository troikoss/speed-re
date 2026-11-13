// Original UG2 PC v1.2 0x587310, 136 bytes. BEHAVIOUR-VERIFIED ONLY (12 clean trials, 28 both-fault):
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
int __cdecl FUN_00741a50();
int __stdcall FUN_005869a0(int,int,int,int,int,int,int);
#define DAT_007a13e8 (*(undefined4 *)0x007a13e8)

int __fastcall FUN_00587310(void *_this,int _edx_unused,int param_1,int param_2,int param_3)

{
  int iVar1;
  undefined4 *puVar2;
  byte local_40;
  undefined4 local_3f;
  
  if ((*(char *)(*(int *)_this + 0x48) == '\0') || (*(char *)((int)_this + 4) != '\0')) {
    return -1;
  }
  FUN_0043dc00((char *)((int)_this + 4),(char *)param_1,0xf);
  local_40 = 0;
  puVar2 = &local_3f;
  for (iVar1 = 0xf; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = 0;
    puVar2 = puVar2 + 1;
  }
  *(undefined2 *)puVar2 = 0;
  *(undefined1 *)((int)puVar2 + 2) = 0;
  FUN_00741a50(&local_40,0x40,&DAT_007a13e8,(byte *)param_1);
  iVar1 = FUN_005869a0(*(void **)_this,0x63706572,(int)&local_40,0x585040,(int)_this,param_2,param_3,
                       '\0');
  return iVar1;
}


