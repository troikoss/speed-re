// Original UG2 PC v1.2 0x6850bd, 110 bytes. BEHAVIOUR-VERIFIED ONLY (39 clean trials, 1 both-fault):
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
int __cdecl FUN_0064307c();
int __cdecl FUN_00643143();
#define DAT_00786e70 (*(undefined4 *)0x00786e70)

undefined4
__stdcall FUN_006850bd(undefined4 param_1,undefined4 param_2,short param_3,int param_4,undefined1 param_5,
            undefined4 *param_6)

{
  if (param_3 == 1) {
LAB_00685104:
    FUN_00643143((void *)*param_6,param_6[0xc],param_4 + 5000,&DAT_00786e70,param_5);
  }
  else {
    if (param_3 != 2) {
      if (param_3 == 5) goto LAB_00685104;
      if (param_3 != 6) {
        return 0;
      }
    }
    FUN_0064307c((void *)*param_6,param_6[0xc],param_4 + 5000,&DAT_00786e70,param_5);
    param_6[0x13] = 1;
  }
  return 0;
}


