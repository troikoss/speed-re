// Original UG2 PC v1.2 0x588b70, 457 bytes. BEHAVIOUR-VERIFIED ONLY (16 clean trials, 24 both-fault):
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
int __cdecl FUN_0043dbb0();
int __cdecl FUN_004400d0();
int __cdecl FUN_00582190();
int __cdecl FUN_00741a50();
int __cdecl FUN_00741da0();
int __stdcall FUN_005869a0(int,int,int,int,int,int,int);
#define DAT_0086993c (*(undefined4 *)0x0086993c)
#define DAT_007a1488 (*(undefined4 *)0x007a1488)
#define DAT_00869888 (*(undefined4 *)0x00869888)
#define DAT_007a1480 (*(undefined4 *)0x007a1480)
#define DAT_008698c8 (*(undefined4 *)0x008698c8)
#define DAT_007a1478 (*(undefined4 *)0x007a1478)
#define DAT_008698cc (*(undefined4 *)0x008698cc)
#define DAT_007a1470 (*(undefined4 *)0x007a1470)
#define DAT_007a145c (*(undefined4 *)0x007a145c)
#define DAT_008698dc (*(undefined4 *)0x008698dc)
#define DAT_00783fe5 (*(undefined4 *)0x00783fe5)
#define s_NAME_d_007a1454 ((char *)0x007a1454)
#define s_NFSUG2_007a1468 ((char *)0x007a1468)
#define s_ROOM_d_007a1444 ((char *)0x007a1444)
#define s_TEAM_d_007a144c ((char *)0x007a144c)
#define s_VENUE_007a1460 ((char *)0x007a1460)

int __fastcall FUN_00588b70(void *_this,int _edx_unused,int param_1,int param_2)

{
  int iVar1;
  byte *pbVar2;
  int iVar3;
  undefined4 *puVar4;
  byte local_978 [8];
  byte local_970;
  undefined4 local_96f;
  
  if (*(char *)((int)_this + 0x48) == '\0') {
    return -1;
  }
  iVar1 = FUN_0043dbb0(&DAT_0086993c);
  if (iVar1 < 1) {
    return 0;
  }
  local_970 = 0;
  puVar4 = &local_96f;
  for (iVar1 = 0x259; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  *(undefined2 *)puVar4 = 0;
  *(undefined1 *)((int)puVar4 + 2) = 0;
  FUN_00741a50(&local_970,0x968,&DAT_007a1488,&DAT_00869888);
  FUN_00741da0(&local_970,0x968,&DAT_007a1480,DAT_008698c8);
  FUN_00741a50(&local_970,0x968,&DAT_007a1478,&DAT_008698cc);
  FUN_00741a50(&local_970,0x968,&DAT_007a1470,&DAT_0086993c);
  FUN_00741a50(&local_970,0x968,(byte *)s_VENUE_007a1460,(byte *)s_NFSUG2_007a1468);
  pbVar2 = (byte *)FUN_00582190();
  FUN_00741a50(&local_970,0x968,&DAT_007a145c,pbVar2);
  iVar1 = 0;
  pbVar2 = &DAT_008698dc;
  do {
    iVar3 = FUN_0043dbb0((char *)pbVar2);
    if (iVar3 < 1) break;
    FUN_004400d0((char *)local_978,s_NAME_d_007a1454,iVar1);
    FUN_00741a50(&local_970,0x968,local_978,pbVar2);
    FUN_004400d0((char *)local_978,s_TEAM_d_007a144c,iVar1);
    FUN_00741a50(&local_970,0x968,local_978,(byte *)s_NFSUG2_007a1468);
    FUN_004400d0((char *)local_978,s_ROOM_d_007a1444,iVar1);
    FUN_00741a50(&local_970,0x968,local_978,&DAT_00783fe5);
    pbVar2 = pbVar2 + 0x10;
    iVar1 = iVar1 + 1;
  } while ((int)pbVar2 < 0x86993c);
  iVar1 = FUN_005869a0(_this,0x72616e6b,(int)&local_970,0x5845a0,(int)_this,param_1,param_2,'\0');
  return iVar1;
}


