// Original UG2 PC v1.2 0x59daf0, 95 bytes. BEHAVIOUR-VERIFIED ONLY (18 clean trials, 22 both-fault):
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



void __fastcall FUN_0059daf0(void *_this,int _edx_unused,float *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  float fVar6;
  float fVar7;
  float fVar8;
  
  fVar1 = *(float *)((int)_this + 0xc4);
  fVar2 = *(float *)((int)_this + 0x128);
  fVar3 = *(float *)((int)_this + 0xb4);
  fVar4 = *(float *)((int)_this + 0x124);
  fVar5 = *(float *)((int)_this + 200);
  fVar6 = *(float *)((int)_this + 0x128);
  fVar7 = *(float *)((int)_this + 0xb8);
  fVar8 = *(float *)((int)_this + 0x124);
  *param_1 = *(float *)((int)_this + 0x124) * *(float *)((int)_this + 0xb0) +
             *(float *)((int)_this + 0xc0) * *(float *)((int)_this + 0x128);
  param_1[1] = fVar3 * fVar4 + fVar1 * fVar2;
  param_1[2] = fVar7 * fVar8 + fVar5 * fVar6;
  return;
}


