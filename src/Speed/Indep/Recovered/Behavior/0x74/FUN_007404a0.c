// Original UG2 PC v1.2 0x7404a0, 347 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_0073fb50();
int __cdecl FUN_0074de50();
int __cdecl FUN_0075c840();


void FUN_007404a0(undefined4 param_1,int param_2,int param_3,int param_4,int param_5)

{
  int *piVar1;
  
  piVar1 = *(int **)(param_2 + 0x100);
  if (piVar1 != (int *)0x0) {
    if (param_3 == 0x70676574) {
      if (*(code **)(param_5 + 0x5c) != (code *)0x0) {
        (**(code **)(param_5 + 0x5c))(param_5,piVar1,param_4,*(undefined4 *)(param_5 + 0x60));
        return;
      }
    }
    else if (param_3 == 0x67696e76) {
      if (param_4 == 0x4000000) {
        *(uint *)(*piVar1 + 0xf8) = *(uint *)(*piVar1 + 0xf8) | 0x4000000;
      }
      else if (param_4 == 8) {
        *(int *)(param_5 + 100) = *(int *)(param_5 + 0x50) + 600000;
        FUN_0075c840((char *)(param_5 + 0x68),(char *)*piVar1,0x20);
      }
      else if ((((param_4 == 0x8000) || (param_4 == 0x800)) || (param_4 == 0x400)) &&
              (*(undefined4 *)(param_5 + 100) = 0, param_4 == 0x400)) {
        FUN_0074de50(*(int *)(param_5 + 8),(byte *)*piVar1,2,0,0,0,*(int *)(param_5 + 0x88));
      }
      else if ((param_4 == 0x400000) && ((*(uint *)((byte *)*piVar1 + 0xf8) & 0x200) != 0)) {
        FUN_0074de50(*(int *)(param_5 + 8),(byte *)*piVar1,1,0,0,0,*(int *)(param_5 + 0x88));
        return;
      }
      if ((*(code **)(param_5 + 0x54) != (code *)0x0) &&
         ((**(code **)(param_5 + 0x54))(param_5,piVar1,param_4,*(undefined4 *)(param_5 + 0x58)),
         param_4 == 0x4000000)) {
        FUN_0073fb50(param_5,*piVar1,2,0,0);
      }
      *(undefined4 *)(param_5 + 0x3c) = 1;
      if (*(code **)(param_5 + 0x24) != (code *)0x0) {
        (**(code **)(param_5 + 0x24))(param_5,5,0,*(undefined4 *)(param_5 + 0x20));
      }
    }
  }
  return;
}


