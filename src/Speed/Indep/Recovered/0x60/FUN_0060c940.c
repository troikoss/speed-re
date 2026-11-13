// Original UG2 PC v1.2 0x60c940, 1304 bytes. Auto-matched: byte-identical to speed2.exe under
// MSVC 7.1 (/Ox /Ob0 dat=undefined4), call targets masked. Source is Ghidra decompiler output with an inferred
// prelude; names, types and callee conventions are NOT verified beyond the bytes.
// Do not build into ug2recovered: globals are absolute-address macros.

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

#define DAT_008a1bf4 (*(undefined4 *)0x008a1bf4)

void __fastcall FUN_0060c940(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar4 = param_1[0xbd];
  iVar5 = param_1[0xc0];
  if (iVar4 != param_1[0x1f7]) {
    param_1[0x1f7] = iVar4;
    param_1[0x1f8] = -1;
  }
  if (iVar4 != param_1[0x1fb]) {
    param_1[0x1fb] = iVar4;
    param_1[0x1fc] = -1;
  }
  if (iVar5 != param_1[0x20b]) {
    param_1[0x20b] = iVar5;
    param_1[0x20c] = -1;
  }
  if (iVar5 != param_1[0x20f]) {
    param_1[0x20f] = iVar5;
    param_1[0x210] = -1;
  }
  if (iVar4 != param_1[0x21f]) {
    param_1[0x21f] = iVar4;
    param_1[0x220] = -1;
  }
  if (iVar4 != param_1[0x223]) {
    param_1[0x223] = iVar4;
    param_1[0x224] = -1;
  }
  if (iVar5 != param_1[0x233]) {
    param_1[0x233] = iVar5;
    param_1[0x234] = -1;
  }
  if (iVar5 != param_1[0x237]) {
    param_1[0x237] = iVar5;
    param_1[0x238] = -1;
  }
  if (iVar4 != param_1[0x31b]) {
    param_1[0x31b] = iVar4;
    param_1[0x31c] = -1;
  }
  if (iVar4 != param_1[799]) {
    param_1[799] = iVar4;
    param_1[800] = -1;
  }
  if (iVar5 != param_1[0x32f]) {
    param_1[0x32f] = iVar5;
    param_1[0x330] = -1;
  }
  if (iVar5 != param_1[0x333]) {
    param_1[0x333] = iVar5;
    param_1[0x334] = -1;
  }
  if (iVar4 != param_1[0x343]) {
    param_1[0x343] = iVar4;
    param_1[0x344] = -1;
  }
  if (iVar4 != param_1[0x347]) {
    param_1[0x347] = iVar4;
    param_1[0x348] = -1;
  }
  if (iVar5 != param_1[0x357]) {
    param_1[0x357] = iVar5;
    param_1[0x358] = -1;
  }
  if (iVar5 != param_1[0x35b]) {
    param_1[0x35b] = iVar5;
    param_1[0x35c] = -1;
  }
  if (iVar4 != param_1[0x43f]) {
    param_1[0x43f] = iVar4;
    param_1[0x440] = -1;
  }
  if (iVar4 != param_1[0x443]) {
    param_1[0x443] = iVar4;
    param_1[0x444] = -1;
  }
  if (iVar5 != param_1[0x453]) {
    param_1[0x453] = iVar5;
    param_1[0x454] = -1;
  }
  if (iVar5 != param_1[0x457]) {
    param_1[0x457] = iVar5;
    param_1[0x458] = -1;
  }
  if (iVar4 != param_1[0x467]) {
    param_1[0x467] = iVar4;
    param_1[0x468] = -1;
  }
  if (iVar4 != param_1[0x46b]) {
    param_1[0x46b] = iVar4;
    param_1[0x46c] = -1;
  }
  if (iVar5 != param_1[0x47b]) {
    param_1[0x47b] = iVar5;
    param_1[0x47c] = -1;
  }
  if (iVar5 != param_1[0x47f]) {
    param_1[0x47f] = iVar5;
    param_1[0x480] = -1;
  }
  iVar4 = 0;
  iVar5 = 0;
  if ((*param_1 != 0) && (*(char *)(*param_1 + 0x944) != '\0')) {
    iVar4 = 1;
    iVar5 = 1;
  }
  if (DAT_008a1bf4 != 0) {
    iVar4 = 1;
    iVar5 = 1;
  }
  iVar1 = param_1[iVar4 + 0xc2];
  iVar2 = param_1[200];
  iVar4 = param_1[iVar4 + 0xc5];
  iVar3 = param_1[iVar5 + 0xc2];
  iVar5 = param_1[iVar5 + 0xc5];
  if (iVar1 != param_1[0x1ff]) {
    param_1[0x1ff] = iVar1;
    param_1[0x200] = -1;
  }
  if (iVar3 != param_1[0x203]) {
    param_1[0x203] = iVar3;
    param_1[0x204] = -1;
  }
  if (iVar2 != param_1[0x207]) {
    param_1[0x207] = iVar2;
    param_1[0x208] = -1;
  }
  if (iVar4 != param_1[0x213]) {
    param_1[0x213] = iVar4;
    param_1[0x214] = -1;
  }
  if (iVar5 != param_1[0x217]) {
    param_1[0x217] = iVar5;
    param_1[0x218] = -1;
  }
  if (iVar2 != param_1[0x21b]) {
    param_1[0x21b] = iVar2;
    param_1[0x21c] = -1;
  }
  if (iVar1 != param_1[0x227]) {
    param_1[0x227] = iVar1;
    param_1[0x228] = -1;
  }
  if (iVar3 != param_1[0x22b]) {
    param_1[0x22b] = iVar3;
    param_1[0x22c] = -1;
  }
  if (iVar2 != param_1[0x22f]) {
    param_1[0x22f] = iVar2;
    param_1[0x230] = -1;
  }
  if (iVar4 != param_1[0x23b]) {
    param_1[0x23b] = iVar4;
    param_1[0x23c] = -1;
  }
  if (iVar5 != param_1[0x23f]) {
    param_1[0x23f] = iVar5;
    param_1[0x240] = -1;
  }
  if (iVar2 != param_1[0x243]) {
    param_1[0x243] = iVar2;
    param_1[0x244] = -1;
  }
  if (iVar1 != param_1[0x323]) {
    param_1[0x323] = iVar1;
    param_1[0x324] = -1;
  }
  if (iVar3 != param_1[0x327]) {
    param_1[0x327] = iVar3;
    param_1[0x328] = -1;
  }
  if (iVar2 != param_1[0x32b]) {
    param_1[0x32b] = iVar2;
    param_1[0x32c] = -1;
  }
  if (iVar4 != param_1[0x337]) {
    param_1[0x337] = iVar4;
    param_1[0x338] = -1;
  }
  if (iVar5 != param_1[0x33b]) {
    param_1[0x33b] = iVar5;
    param_1[0x33c] = -1;
  }
  if (iVar2 != param_1[0x33f]) {
    param_1[0x33f] = iVar2;
    param_1[0x340] = -1;
  }
  if (iVar1 != param_1[0x34b]) {
    param_1[0x34b] = iVar1;
    param_1[0x34c] = -1;
  }
  if (iVar3 != param_1[0x34f]) {
    param_1[0x34f] = iVar3;
    param_1[0x350] = -1;
  }
  if (iVar2 != param_1[0x353]) {
    param_1[0x353] = iVar2;
    param_1[0x354] = -1;
  }
  if (iVar4 != param_1[0x35f]) {
    param_1[0x35f] = iVar4;
    param_1[0x360] = -1;
  }
  if (iVar5 != param_1[0x363]) {
    param_1[0x363] = iVar5;
    param_1[0x364] = -1;
  }
  if (iVar2 != param_1[0x367]) {
    param_1[0x367] = iVar2;
    param_1[0x368] = -1;
  }
  if (iVar1 != param_1[0x447]) {
    param_1[0x447] = iVar1;
    param_1[0x448] = -1;
  }
  if (iVar3 != param_1[1099]) {
    param_1[1099] = iVar3;
    param_1[0x44c] = -1;
  }
  if (iVar2 != param_1[0x44f]) {
    param_1[0x44f] = iVar2;
    param_1[0x450] = -1;
  }
  if (iVar4 != param_1[0x45b]) {
    param_1[0x45b] = iVar4;
    param_1[0x45c] = -1;
  }
  if (iVar5 != param_1[0x45f]) {
    param_1[0x45f] = iVar5;
    param_1[0x460] = -1;
  }
  if (iVar2 != param_1[0x463]) {
    param_1[0x463] = iVar2;
    param_1[0x464] = -1;
  }
  if (iVar1 != param_1[0x46f]) {
    param_1[0x46f] = iVar1;
    param_1[0x470] = -1;
  }
  if (iVar3 != param_1[0x473]) {
    param_1[0x473] = iVar3;
    param_1[0x474] = -1;
  }
  if (iVar2 != param_1[0x477]) {
    param_1[0x477] = iVar2;
    param_1[0x478] = -1;
  }
  if (iVar4 != param_1[0x483]) {
    param_1[0x483] = iVar4;
    param_1[0x484] = -1;
  }
  if (iVar5 != param_1[0x487]) {
    param_1[0x487] = iVar5;
    param_1[0x488] = -1;
  }
  if (iVar2 != param_1[0x48b]) {
    param_1[0x48b] = iVar2;
    param_1[0x48c] = -1;
  }
  return;
}


