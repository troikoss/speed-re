// Original UG2 PC v1.2 0x4a9b80, 110 bytes. BEHAVIOUR-VERIFIED ONLY (21 clean trials, 19 both-fault):
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
int __cdecl FUN_0057d670();
int __cdecl FUN_0057e480();
#define DAT_008650d4 (*(undefined4 *)0x008650d4)
#define DAT_00836498 (*(undefined4 *)0x00836498)

undefined4 __stdcall FUN_004a9b80(uint param_1,uint param_2,int *param_3,undefined4 *param_4)

{
  uint uVar1;
  uint uVar2;
  undefined4 uVar3;
  
  if (DAT_008650d4 != 0) {
    FUN_0057d670(0x11,4,8);
  }
  uVar1 = FUN_0057e480(param_1,0x20,8);
  uVar2 = FUN_0057e480(param_2,0x20,8);
  uVar3 = 1;
  if ((uVar1 == 8) && (uVar2 == *(uint *)(DAT_00836498 + 0x54))) {
    *param_4 = *(undefined4 *)(DAT_00836498 + 0x1c);
    *param_3 = DAT_00836498 + 0x28;
    uVar3 = 0;
  }
  return uVar3;
}


