// Original UG2 PC v1.2 0x72ab71, 63 bytes. BEHAVIOUR-VERIFIED ONLY (17 clean trials, 23 both-fault):
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
int __cdecl FUN_0072d8e9();
#define DAT_008b466c (*(undefined4 *)0x008b466c)

undefined4 FUN_0072ab71(int param_1,undefined4 *param_2)

{
  int iVar1;
  int iVar2;
  short *psVar3;
  
  iVar2 = param_1 * 0x90 + DAT_008b466c;
  iVar1 = 0;
  if (*(char *)(iVar2 + 0x23) != '\0') {
    psVar3 = (short *)(iVar2 + 4);
    do {
      FUN_0072d8e9((int)*psVar3,iVar1,param_2);
      iVar1 = iVar1 + 1;
      psVar3 = psVar3 + 1;
    } while (iVar1 < (int)(uint)*(byte *)(iVar2 + 0x23));
  }
  return 0;
}


