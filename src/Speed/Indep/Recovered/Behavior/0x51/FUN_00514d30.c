// Original UG2 PC v1.2 0x514d30, 75 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int useCachedFlag;

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



int __fastcall FUN_00514d30(void *_this,int _edx_unused,int useCachedFlag)

{
  int iVar1;
  int iVar2;
  void *pvVar3;
  int iVar4;
  
  if ((char)useCachedFlag != '\0') {
    return *(int *)((int)_this + 0x1404);
  }
  iVar1 = *(int *)((int)_this + 0x1400);
  iVar2 = 0;
  iVar4 = 0;
  if (0 < iVar1) {
    do {
      if ((iVar4 < 0) || (iVar1 <= iVar4)) {
        pvVar3 = (void *)0x0;
      }
      else {
        pvVar3 = _this;
      }
      if ((*(char *)((int)pvVar3 + 4) == '\0') && (*(char *)((int)pvVar3 + 6) != '\x01')) {
        iVar2 = iVar2 + 1;
      }
      iVar4 = iVar4 + 1;
      _this = (void *)((int)_this + 0x14);
    } while (iVar4 < iVar1);
  }
  return iVar2;
}


