// Original UG2 PC v1.2 0x75f96e, 129 bytes. BEHAVIOUR-VERIFIED ONLY (26 clean trials, 14 both-fault):
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
int __cdecl FUN_00766bc0();
int __cdecl FUN_0076b9f8();
int __cdecl FUN_0076ba46();
#define DAT_008bb5e4 (*(undefined4 *)0x008bb5e4)
#define DAT_008b8450 (*(undefined4 *)0x008b8450)
#define DAT_008b8458 (*(undefined4 *)0x008b8458)

/* Library Function - Single Match
    __getenv_lk
   
   Library: Visual Studio 2003 Release */

int FUN_0075f96e(uchar *param_1)

{
  int iVar1;
  size_t _MaxCount;
  size_t sVar2;
  int *piVar3;
  
  if (DAT_008bb5e4 == 0) {
    return 0;
  }
  if (((DAT_008b8450 != (int *)0x0) ||
      (((DAT_008b8458 != 0 && (iVar1 = FUN_0076ba46(), iVar1 == 0)) && (DAT_008b8450 != (int *)0x0))
      )) && (piVar3 = DAT_008b8450, param_1 != (uchar *)0x0)) {
    _MaxCount = FUN_00766bc0((char *)param_1);
    for (; (char *)*piVar3 != (char *)0x0; piVar3 = piVar3 + 1) {
      sVar2 = FUN_00766bc0((char *)*piVar3);
      if (((_MaxCount < sVar2) && (((uchar *)*piVar3)[_MaxCount] == '=')) &&
         (iVar1 = FUN_0076b9f8((uchar *)*piVar3,param_1,_MaxCount), iVar1 == 0)) {
        return *piVar3 + 1 + _MaxCount;
      }
    }
  }
  return 0;
}


