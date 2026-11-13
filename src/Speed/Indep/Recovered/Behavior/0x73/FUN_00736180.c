// Original UG2 PC v1.2 0x736180, 219 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int address;

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
int __cdecl FUN_00735a30();
int __cdecl FUN_007364d0();
int __cdecl FUN_00736560();
int __cdecl FUN_00736a60();
int __cdecl FUN_00737fc0();
int __cdecl FUN_00738620();
int __cdecl FUN_00738940();
#define DAT_008b5160 (*(undefined4 *)0x008b5160)
#define DAT_008b5148 (*(undefined4 *)0x008b5148)
#define DAT_008b514c (*(undefined4 *)0x008b514c)
#define DAT_008b5150 (*(undefined4 *)0x008b5150)
#define DAT_008b5154 (*(undefined4 *)0x008b5154)
#define DAT_008b5158 (*(undefined4 *)0x008b5158)
#define DAT_008b515c (*(undefined4 *)0x008b515c)
#define DAT_008b4f5c (*(undefined4 *)0x008b4f5c)
#define _DAT_008b5184 (*(undefined4 *)0x008b5184)
#define _DAT_008b4f60 (*(undefined4 *)0x008b4f60)
#define _DAT_008b4f64 (*(undefined4 *)0x008b4f64)
#define _DAT_008b4f68 (*(undefined4 *)0x008b4f68)
#define _DAT_008b4f6c (*(undefined4 *)0x008b4f6c)
#define _DAT_008b4f70 (*(undefined4 *)0x008b4f70)
#define _DAT_008b4f74 (*(undefined4 *)0x008b4f74)
#define _DAT_008b4f78 (*(undefined4 *)0x008b4f78)
#define _DAT_008b4f7c (*(undefined4 *)0x008b4f7c)
#define DAT_008b4f8c (*(undefined4 *)0x008b4f8c)
#define DAT_008b5170 (*(undefined4 *)0x008b5170)
#define s_oSPCHAUTHOR__TLBuild__Monday_04__00819acb ((char *)0x00819acb)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00736180(undefined4 param_1,int param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  
  s_oSPCHAUTHOR__TLBuild__Monday_04__00819acb[1] = 'S';
  if (DAT_008b5160 == 0) {
    return 0;
  }
  DAT_008b5148 = param_1;
  DAT_008b514c = 0;
  DAT_008b5150 = 0;
  DAT_008b5154 = 0;
  DAT_008b5158 = 0;
  DAT_008b515c = param_2;
  DAT_008b4f5c = param_3;
  _DAT_008b5184 = FUN_00736a60;
  FUN_00738940(param_2);
  FUN_00737fc0();
  FUN_00738620(0);
  _DAT_008b4f60 = 0;
  _DAT_008b4f64 = 0;
  _DAT_008b4f68 = 0;
  _DAT_008b4f6c = 0;
  _DAT_008b4f70 = 0;
  _DAT_008b4f74 = 0;
  _DAT_008b4f78 = 0;
  _DAT_008b4f7c = 0;
  FUN_007364d0();
  puVar1 = &DAT_008b4f8c;
  do {
    *(undefined2 *)(puVar1 + -1) = 0xffff;
    *puVar1 = 0;
    puVar1 = puVar1 + 2;
  } while ((int)puVar1 < 0x8b4fcc);
  FUN_00735a30();
  FUN_00736560();
  DAT_008b5170 = 0x1789a34;
  return 1;
}


