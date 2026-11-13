// Original UG2 PC v1.2 0x40a100, 68 bytes. BEHAVIOUR-VERIFIED ONLY (6 clean trials, 34 both-fault):
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

#define DAT_007d7d3c (*(float *)0x007d7d3c)
#define DAT_00783c1c (*(float *)0x00783c1c)

void __fastcall FUN_0040a100(void *_this,int _edx_unused,float param_1)

{
  float fVar1;
  float fVar2;
  
  fVar2 = DAT_007d7d3c;
  fVar1 = *(float *)((int)_this + 0x54) - param_1;
  *(float *)((int)_this + 0x54) = fVar1;
  if (fVar1 <= fVar2) {
    *(float *)((int)_this + 0x58) =
         (*(float *)(*(int *)((int)_this + 4) + 0x430) - *(float *)((int)_this + 0x5c)) /
         (DAT_00783c1c - fVar1);
                    /* WARNING: Could not recover jumptable at 0x0040a13c. Too many branches */
                    /* WARNING: Treating indirect jump as call */
    (**(code **)(*(int *)_this + 0x30))();
    return;
  }
  return;
}


