// Original UG2 PC v1.2 0x737fc0, 85 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008b518c (*(undefined4 *)0x008b518c)

void FUN_00737fc0(void)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  int iVar3;
  
  puVar2 = &DAT_008b518c;
  do {
    puVar2[-1] = 0;
    *puVar2 = 0;
    *(undefined1 *)(puVar2 + 1) = 0;
    *(undefined1 *)((int)puVar2 + 5) = 0;
    *(undefined1 *)((int)puVar2 + 6) = 0;
    *(undefined1 *)((int)puVar2 + 7) = 0;
    puVar2[2] = 0;
    puVar1 = puVar2 + 4;
    iVar3 = 0xc;
    do {
      puVar1[-1] = 0xffffffff;
      *(undefined2 *)puVar1 = 0xffff;
      *(undefined2 *)((int)puVar1 + 2) = 0xffff;
      *(undefined2 *)(puVar1 + 1) = 0;
      puVar1 = puVar1 + 3;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    puVar2 = puVar2 + 0x28;
  } while ((int)puVar2 < 0x8b568c);
  return;
}


