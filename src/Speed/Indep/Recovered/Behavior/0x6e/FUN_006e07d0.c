// Original UG2 PC v1.2 0x6e07d0, 112 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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
int __cdecl FUN_006dbf90();
int __cdecl FUN_006dc330();
int __cdecl FUN_006e04f0();
#define DAT_008a7a10 (*(undefined4 *)0x008a7a10)
#define DAT_008a88a4 (*(undefined4 *)0x008a88a4)
#define DAT_008a7a18 (*(undefined4 *)0x008a7a18)
#define DAT_008a8a80 (*(undefined4 *)0x008a8a80)
#define DAT_008a88a0 (*(undefined4 *)0x008a88a0)
#define DAT_008a8a1c (*(undefined4 *)0x008a8a1c)
#define DAT_008a8a20 (*(undefined4 *)0x008a8a20)
#define DAT_008a8a24 (*(undefined4 *)0x008a8a24)

void FUN_006e07d0(void)

{
  char cVar1;
  int iVar2;
  
  cVar1 = FUN_006dc330();
  if (cVar1 != '\0') {
    iVar2 = *(int *)(DAT_008a7a10 + 8) >> 2;
    if (DAT_008a88a4 < iVar2) {
      iVar2 = DAT_008a88a4;
    }
    FUN_006e04f0(*(int *)(DAT_008a7a18 + 4),(undefined8 *)&DAT_008a8a80,DAT_008a88a0,DAT_008a8a1c,
                 DAT_008a8a20,DAT_008a8a24);
    FUN_006dbf90(*(undefined4 *)(DAT_008a7a10 + 4),&DAT_008a8a80,iVar2 * 4);
  }
  return;
}


