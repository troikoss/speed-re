// Original UG2 PC v1.2 0x430d40, 101 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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
int __cdecl FUN_0043dc30();
#define s_CAR_WHEEL_FL_00785fa4 ((char *)0x00785fa4)
#define s_CAR_WHEEL_FR_00785f94 ((char *)0x00785f94)
#define s_CAR_WHEEL_RL_00785f74 ((char *)0x00785f74)
#define s_CAR_WHEEL_RR_00785f84 ((char *)0x00785f84)

int FUN_00430d40(char *param_1)

{
  int iVar1;
  
  iVar1 = FUN_0043dc30(param_1,s_CAR_WHEEL_FL_00785fa4);
  if (iVar1 == 0) {
    return 0;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_WHEEL_FR_00785f94);
  if (iVar1 == 0) {
    return 1;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_WHEEL_RR_00785f84);
  if (iVar1 == 0) {
    return 2;
  }
  iVar1 = FUN_0043dc30(param_1,s_CAR_WHEEL_RL_00785f74);
  return (-(uint)(iVar1 != 0) & 0xfffffffc) + 3;
}


