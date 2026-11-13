// Original UG2 PC v1.2 0x582d40, 62 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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


void __fastcall FUN_00582d40(void *_this,int _edx_unused,char *param_1)

{
  int iVar1;
  int iVar2;
  undefined4 *puVar3;
  undefined4 *puVar4;
  
  puVar3 = (undefined4 *)((int)_this + 0x110);
  iVar2 = 6;
  do {
    iVar1 = FUN_0043dc30(param_1,(char *)((int)puVar3 + 0x21));
    if (iVar1 == 0) {
      puVar4 = puVar3;
      for (iVar1 = 0x30; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar4 = 0;
        puVar4 = puVar4 + 1;
      }
      *(undefined1 *)puVar4 = 0;
    }
    puVar3 = (undefined4 *)((int)puVar3 + 0xc1);
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  return;
}


