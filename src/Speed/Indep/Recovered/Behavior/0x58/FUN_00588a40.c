// Original UG2 PC v1.2 0x588a40, 179 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_00741a50();
int __cdecl FUN_00742c80();
int __stdcall FUN_00503260(int);
int __stdcall FUN_005869a0(int,int,int,int,int,int,int);
#define DAT_00850080 (*(undefined4 *)0x00850080)
#define DAT_007a13e8 (*(undefined4 *)0x007a13e8)
#define DAT_00790b60 (*(undefined4 *)0x00790b60)

int __fastcall FUN_00588a40(void *_this,int _edx_unused,int param_1,int param_2,int param_3)

{
  byte *pbVar1;
  int iVar2;
  undefined4 *puVar3;
  byte local_108;
  undefined4 local_107;
  
  if ((*(char *)((int)_this + 0x48) == '\0') ||
     (pbVar1 = (byte *)((param_1 + 5) * 0x10 + (int)_this), *pbVar1 == 0)) {
    return -1;
  }
  FUN_00503260(&DAT_00850080,(int)pbVar1);
  local_108 = 0;
  puVar3 = &local_107;
  for (iVar2 = 0x3f; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  *(undefined2 *)puVar3 = 0;
  *(undefined1 *)((int)puVar3 + 2) = 0;
  FUN_00741a50(&local_108,0x100,&DAT_007a13e8,pbVar1);
  pbVar1 = FUN_00742c80();
  FUN_00741a50(&local_108,0x100,&DAT_00790b60,pbVar1);
  iVar2 = FUN_005869a0(_this,0x70657273,(int)&local_108,0x584520,(int)_this,param_2,param_3,'\0');
  return iVar2;
}


