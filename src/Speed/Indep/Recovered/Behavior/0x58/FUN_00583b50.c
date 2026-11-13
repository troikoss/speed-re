// Original UG2 PC v1.2 0x583b50, 186 bytes. BEHAVIOUR-VERIFIED ONLY (8 clean trials, 32 both-fault):
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
int __cdecl FUN_0043dc00();
int __cdecl FUN_00748ba0();
int __cdecl FUN_00748c60();


bool __fastcall FUN_00583b50(void *_this,int _edx_unused,int *param_1)

{
  bool bVar1;
  
  bVar1 = *(int *)((int)_this + 4) != 0;
  if (bVar1) {
    FUN_0043dc00((char *)((int)_this + 0xc),(char *)*param_1,0x13);
    FUN_0043dc00((char *)((int)_this + 0x40),(char *)param_1[1],0x1f);
    FUN_0043dc00((char *)((int)_this + 0x60),(char *)param_1[2],0x1f);
    FUN_0043dc00((char *)((int)_this + 0x80),(char *)param_1[3],3);
    FUN_0043dc00((char *)((int)_this + 0x84),(char *)param_1[4],3);
    FUN_0043dc00((char *)((int)_this + 0x88),(char *)param_1[5],0x1f);
    FUN_0043dc00((char *)((int)_this + 0xa8),(char *)param_1[6],0x27);
    *(undefined1 *)((int)_this + 0x20) = 0;
    FUN_00748ba0(*(int *)((int)_this + 4),(byte *)((int)_this + 0xc));
    FUN_00748c60(*(undefined4 **)((int)_this + 4),(char *)param_1[7]);
    *(undefined1 *)((int)_this + 0xd0) = 0;
  }
  return bVar1;
}


