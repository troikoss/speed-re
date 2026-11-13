// Original UG2 PC v1.2 0x587230, 222 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_005712a0();
int __cdecl FUN_00571380();
int __cdecl FUN_00741a50();
int __stdcall FUN_005869a0(int,int,int,int,int,int,int);
#define DAT_0079ac28 (*(undefined4 *)0x0079ac28)
#define DAT_007a13a0 (*(undefined4 *)0x007a13a0)
#define DAT_007a1398 (*(undefined4 *)0x007a1398)

int __fastcall FUN_00587230(void *_this,int _edx_unused,char *param_1,int param_2,int param_3)

{
  char *pcVar1;
  byte *pbVar2;
  undefined *puVar3;
  int iVar4;
  undefined4 *puVar5;
  byte local_108;
  undefined4 local_107;
  
  if (((*(char *)(*(int *)_this + 0x48) == '\0') || (param_1 == (char *)0x0)) || (*param_1 == '\0'))
  {
    return -1;
  }
  local_108 = 0;
  puVar5 = &local_107;
  for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar5 = 0;
    puVar5 = puVar5 + 1;
  }
  *(undefined2 *)puVar5 = 0;
  *(undefined1 *)((int)puVar5 + 2) = 0;
  FUN_00741a50(&local_108,0x100,&DAT_0079ac28,(byte *)param_1);
  pcVar1 = FUN_00571380();
  if (pcVar1 != (char *)0x0) {
    pbVar2 = (byte *)FUN_00571380();
    FUN_00741a50(&local_108,0x100,&DAT_007a13a0,pbVar2);
  }
  puVar3 = FUN_005712a0();
  if (puVar3 != (undefined *)0x0) {
    pbVar2 = FUN_005712a0();
    FUN_00741a50(&local_108,0x100,&DAT_007a1398,pbVar2);
  }
  iVar4 = FUN_005869a0(*(void **)_this,0x6c6f7374,(int)&local_108,0x584630,(int)*(void **)_this,
                       param_2,param_3,'\0');
  return iVar4;
}


