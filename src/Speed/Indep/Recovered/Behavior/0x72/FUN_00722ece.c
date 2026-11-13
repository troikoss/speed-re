// Original UG2 PC v1.2 0x722ece, 170 bytes. BEHAVIOUR-VERIFIED ONLY (14 clean trials, 26 both-fault):
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
int __cdecl FUN_00722424();
int __cdecl FUN_00723649();
int __cdecl FUN_00727a12();
int __cdecl FUN_00770e48();
#define DAT_008b4608 (*(undefined4 *)0x008b4608)

undefined4 FUN_00722ece(int param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  uint extraout_EAX;
  undefined4 uVar5;
  
  param_2[2] = 0;
  param_2[1] = 0;
  *param_2 = 0;
  if (DAT_008b4608 == '\0') {
    uVar5 = 0xfffffff6;
  }
  else {
    iVar1 = FUN_00722424(param_1);
    if (iVar1 == 0) {
      uVar5 = 0xfffffff8;
    }
    else {
      iVar2 = *(int *)(iVar1 + 0x14c);
      *param_2 = iVar2;
      if ((iVar2 != 0) &&
         (param_2[1] = *(int *)(*(int *)(iVar1 + 0x144) + 0xc), *(short *)(iVar1 + 0x38) != 0)) {
        iVar2 = FUN_00723649(*(int *)(iVar1 + 8));
        uVar3 = (uint)(iVar2 * 1000) / (uint)*(ushort *)(iVar1 + 0x38);
        param_2[2] = uVar3;
        if ((uVar3 == 0) && (puVar4 = FUN_00727a12(param_2[1]), puVar4[4] != 0)) {
          FUN_00770e48();
          uVar3 = extraout_EAX;
          if (4000000 < extraout_EAX) {
            uVar3 = 4000000;
          }
          param_2[2] = (uVar3 * 1000) / (uint)puVar4[4];
        }
      }
      uVar5 = 0;
    }
  }
  return uVar5;
}


