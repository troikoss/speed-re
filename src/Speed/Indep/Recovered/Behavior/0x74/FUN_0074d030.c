// Original UG2 PC v1.2 0x74d030, 131 bytes. BEHAVIOUR-VERIFIED ONLY (13 clean trials, 27 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int a;

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
int __cdecl FUN_00741650();


undefined4
FUN_0074d030(int param_1,char *param_2,undefined4 param_3,int param_4,undefined4 param_5,
            undefined4 param_6)

{
  int iVar1;
  ushort uVar2;
  ushort *puVar3;
  
  FUN_00741650((char *)(param_1 + 0x4e8),0x40,param_2);
  *(undefined4 *)(param_1 + 0x1b0) = param_3;
  iVar1 = 0;
  puVar3 = (ushort *)(param_1 + 0x52c);
  do {
    if ((*(char *)(param_4 + iVar1 * 2) < 'a') || (*(char *)(param_4 + 1 + iVar1 * 2) < 'a')) break;
    *puVar3 = 0;
    uVar2 = (short)*(char *)(param_4 + iVar1 * 2) << 8;
    *puVar3 = uVar2;
    *puVar3 = (short)*(char *)(param_4 + 1 + iVar1 * 2) | uVar2;
    iVar1 = iVar1 + 1;
    puVar3 = puVar3 + 1;
  } while (iVar1 < 4);
  *(int *)(param_1 + 0x528) = iVar1;
  *(undefined4 *)(param_1 + 0x1a8) = param_6;
  *(undefined4 *)(param_1 + 0x20) = param_5;
  return 0;
}


