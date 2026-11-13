// Original UG2 PC v1.2 0x586350, 292 bytes. BEHAVIOUR-VERIFIED ONLY (40 clean trials, 0 both-fault):
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

#define DAT_008693cc (*(undefined4 *)0x008693cc)
#define DAT_008693c8 (*(undefined4 *)0x008693c8)
#define _DAT_008693d0 (*(undefined4 *)0x008693d0)
#define _DAT_008693d4 (*(undefined4 *)0x008693d4)
#define DAT_008693d6 (*(undefined4 *)0x008693d6)
#define DAT_008693d8 (*(undefined4 *)0x008693d8)
#define _DAT_008693dc (*(undefined4 *)0x008693dc)
#define _DAT_008693e0 (*(undefined4 *)0x008693e0)
#define DAT_008693e4 (*(undefined4 *)0x008693e4)
#define DAT_008651ac (*(undefined4 *)0x008651ac)
#define DAT_008693ec (*(undefined4 *)0x008693ec)
#define DAT_008693e8 (*(undefined4 *)0x008693e8)
#define DAT_008693f0 (*(undefined4 *)0x008693f0)
#define DAT_008693f4 (*(undefined4 *)0x008693f4)
#define DAT_008693f6 (*(undefined4 *)0x008693f6)
#define DAT_008693f8 (*(undefined4 *)0x008693f8)
#define DAT_008693fc (*(undefined4 *)0x008693fc)
#define DAT_00869400 (*(undefined4 *)0x00869400)
#define DAT_00869404 (*(undefined4 *)0x00869404)
#define DAT_0086940c (*(undefined4 *)0x0086940c)
#define _DAT_00869408 (*(undefined4 *)0x00869408)
#define _DAT_00869410 (*(undefined4 *)0x00869410)
#define _DAT_00869414 (*(undefined4 *)0x00869414)
#define DAT_00869416 (*(undefined4 *)0x00869416)
#define DAT_00869418 (*(undefined4 *)0x00869418)
#define _DAT_0086941c (*(undefined4 *)0x0086941c)
#define _DAT_00869420 (*(undefined4 *)0x00869420)
#define _DAT_00869424 (*(undefined4 *)0x00869424)
#define DAT_0086942c (*(undefined4 *)0x0086942c)
#define _DAT_00869428 (*(undefined4 *)0x00869428)
#define _DAT_00869430 (*(undefined4 *)0x00869430)
#define _DAT_00869434 (*(undefined4 *)0x00869434)
#define DAT_00869436 (*(undefined4 *)0x00869436)
#define DAT_00869438 (*(undefined4 *)0x00869438)
#define _DAT_0086943c (*(undefined4 *)0x0086943c)
#define _DAT_00869440 (*(undefined4 *)0x00869440)
#define _DAT_00869444 (*(undefined4 *)0x00869444)
#define DAT_0086944c (*(undefined4 *)0x0086944c)
#define _DAT_00869448 (*(undefined4 *)0x00869448)
#define _DAT_00869450 (*(undefined4 *)0x00869450)
#define _DAT_00869454 (*(undefined4 *)0x00869454)
#define DAT_00869456 (*(undefined4 *)0x00869456)
#define DAT_00869458 (*(undefined4 *)0x00869458)
#define _DAT_0086945c (*(undefined4 *)0x0086945c)
#define _DAT_00869460 (*(undefined4 *)0x00869460)
#define _DAT_00869464 (*(undefined4 *)0x00869464)
#define DAT_00866a24 (*(undefined4 *)0x00866a24)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00586350(void)

{
  DAT_008693cc = 0xffffffff;
  DAT_008693c8 = 0xffffffff;
  _DAT_008693d0 = 0;
  _DAT_008693d4 = 0;
  DAT_008693d6 = 0;
  DAT_008693d8 = 0;
  _DAT_008693dc = 0;
  _DAT_008693e0 = 0xbf800000;
  DAT_008693e4 = DAT_008651ac;
  DAT_008693ec = 0xffffffff;
  DAT_008693e8 = 0xffffffff;
  DAT_008693f0 = 0;
  DAT_008693f4 = 0;
  DAT_008693f6 = 0;
  DAT_008693f8 = 0;
  DAT_008693fc = 0;
  DAT_00869400 = 0xbf800000;
  DAT_00869404 = DAT_008651ac;
  DAT_0086940c = 0xffffffff;
  _DAT_00869408 = 0xffffffff;
  _DAT_00869410 = 0;
  _DAT_00869414 = 0;
  DAT_00869416 = 0;
  DAT_00869418 = 0;
  _DAT_0086941c = 0;
  _DAT_00869420 = 0xbf800000;
  _DAT_00869424 = DAT_008651ac;
  DAT_0086942c = 0xffffffff;
  _DAT_00869428 = 0xffffffff;
  _DAT_00869430 = 0;
  _DAT_00869434 = 0;
  DAT_00869436 = 0;
  DAT_00869438 = 0;
  _DAT_0086943c = 0;
  _DAT_00869440 = 0xbf800000;
  _DAT_00869444 = DAT_008651ac;
  DAT_0086944c = 0xffffffff;
  _DAT_00869448 = 0xffffffff;
  _DAT_00869450 = 0;
  _DAT_00869454 = 0;
  DAT_00869456 = 0;
  DAT_00869458 = 0;
  _DAT_0086945c = 0;
  _DAT_00869460 = 0xbf800000;
  _DAT_00869464 = DAT_008651ac;
  DAT_00866a24 = 0;
  return;
}


