// Original UG2 PC v1.2 0x497810, 64 bytes. BEHAVIOUR-VERIFIED ONLY (9 clean trials, 31 both-fault):
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
int __cdecl FUN_00537980();
#define DAT_007f4434 (*(undefined4 *)0x007f4434)
#define DAT_00836334 (*(undefined4 *)0x00836334)
#define s_UI_EngageMessageDialog_fng_0078f7c8 ((char *)0x0078f7c8)
#define s_UI_SMS_Mailbox_fng_0078f7e4 ((char *)0x0078f7e4)

void __fastcall FUN_00497810(int param_1)

{
  int iVar1;
  
  if (((*(int *)(param_1 + 0x60) != param_1 + 0x60) && (*(int *)(param_1 + 0x74) != 0)) &&
     (iVar1 = *(int *)(*(int *)(param_1 + 0x74) + 0x18), iVar1 != 0)) {
    DAT_007f4434 = *(undefined4 *)(*(int *)(param_1 + 0x120) + 0x18);
    DAT_00836334 = iVar1;
    FUN_00537980(s_UI_SMS_Mailbox_fng_0078f7e4,s_UI_EngageMessageDialog_fng_0078f7c8,iVar1);
  }
  return;
}


