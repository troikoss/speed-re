// Original UG2 PC v1.2 0x749ae0, 353 bytes. Auto-matched: byte-identical to speed2.exe under
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
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef void *HANDLE;
typedef unsigned int size_t;
typedef int BOOL;
int __cdecl FUN_00741690();
int __cdecl FUN_00741a50();
#define DAT_007d2ce8 (*(undefined4 *)0x007d2ce8)
#define s_ADDRESS1_007d2cfc ((char *)0x007d2cfc)
#define s_ADDRESS2_007d2cf0 ((char *)0x007d2cf0)
#define s_CARDNUMBER_007d2cac ((char *)0x007d2cac)
#define s_CARDTYPE_007d2cb8 ((char *)0x007d2cb8)
#define s_COUNTRY_007d2ccc ((char *)0x007d2ccc)
#define s_EMAIL_007d2cc4 ((char *)0x007d2cc4)
#define s_EXPIRYMONTH_007d2ca0 ((char *)0x007d2ca0)
#define s_EXPIRYYEAR_007d2c94 ((char *)0x007d2c94)
#define s_FIRSTNAME_007d2d20 ((char *)0x007d2d20)
#define s_LASTNAME_007d2d14 ((char *)0x007d2d14)
#define s_NAMEONCARD_007d2d08 ((char *)0x007d2d08)
#define s_POSTCODE_007d2cd4 ((char *)0x007d2cd4)
#define s_STATE_007d2ce0 ((char *)0x007d2ce0)

void FUN_00749ae0(int param_1,int param_2)

{
  byte *pbVar1;
  
  pbVar1 = (byte *)(param_1 + 0x110);
  FUN_00741a50(pbVar1,0x400,(byte *)s_FIRSTNAME_007d2d20,(byte *)(param_2 + 0x2f8));
  FUN_00741a50(pbVar1,0x400,(byte *)s_LASTNAME_007d2d14,(byte *)(param_2 + 0x32b));
  FUN_00741a50(pbVar1,0x400,(byte *)s_NAMEONCARD_007d2d08,(byte *)(param_2 + 0x35e));
  FUN_00741a50(pbVar1,0x400,(byte *)s_ADDRESS1_007d2cfc,(byte *)(param_2 + 0x3c4));
  FUN_00741a50(pbVar1,0x400,(byte *)s_ADDRESS2_007d2cf0,(byte *)(param_2 + 0x3f7));
  FUN_00741a50(pbVar1,0x400,&DAT_007d2ce8,(byte *)(param_2 + 0x42a));
  FUN_00741a50(pbVar1,0x400,(byte *)s_STATE_007d2ce0,(byte *)(param_2 + 0x45d));
  FUN_00741a50(pbVar1,0x400,(byte *)s_POSTCODE_007d2cd4,(byte *)(param_2 + 0x47c));
  FUN_00741a50(pbVar1,0x400,(byte *)s_COUNTRY_007d2ccc,(byte *)(param_2 + 0x487));
  FUN_00741a50(pbVar1,0x400,(byte *)s_EMAIL_007d2cc4,(byte *)(param_2 + 0x4ba));
  FUN_00741a50(pbVar1,0x400,(byte *)s_CARDTYPE_007d2cb8,(byte *)(param_2 + 0x4ed));
  FUN_00741a50(pbVar1,0x400,(byte *)s_CARDNUMBER_007d2cac,(byte *)(param_2 + 0x501));
  FUN_00741690(pbVar1,0x400,(byte *)s_EXPIRYMONTH_007d2ca0,*(int *)(param_2 + 0x518));
  FUN_00741690(pbVar1,0x400,(byte *)s_EXPIRYYEAR_007d2c94,*(int *)(param_2 + 0x51c));
  return;
}


