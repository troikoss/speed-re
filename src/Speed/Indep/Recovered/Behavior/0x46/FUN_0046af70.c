// Original UG2 PC v1.2 0x46af70, 362 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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
int __cdecl FUN_00468160();
int __cdecl FUN_0075bed4();
#define _DAT_00784250 (*(undefined4 *)0x00784250)
#define _DAT_0078a6c8 (*(undefined4 *)0x0078a6c8)

/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0046af70(void *_this,int _edx_unused,float param_1)

{
  int iVar1;
  float fVar2;
  float fVar3;
  int iVar4;
  uint uVar5;
  float10 extraout_ST0;
  ulonglong uVar6;
  
  if ((*(char *)((int)_this + 0x78) == '\0') && (*(int *)((int)_this + 0x6c) != 0)) {
    iVar4 = *(int *)((int)_this + 0x70);
    fVar3 = param_1 + *(float *)_this;
    *(float *)_this = fVar3;
    if (fVar3 <= *(float *)((int)_this + iVar4 * 4 + 4)) {
      switch(*(undefined4 *)((int)_this + iVar4 * 4 + 0x54)) {
      case 1:
        break;
      case 2:
        fVar3 = fVar3 / *(float *)((int)_this + iVar4 * 4 + 4);
        *(float *)((int)_this + 0x74) =
             (*(float *)((int)_this + iVar4 * 4 + 0x34) - *(float *)((int)_this + iVar4 * 4 + 0x1c)) *
             fVar3 * fVar3 + *(float *)((int)_this + iVar4 * 4 + 0x1c);
        return;
      case 3:
        fVar3 = (_DAT_00784250 / *(float *)((int)_this + iVar4 * 4 + 4)) * fVar3 - _DAT_00784250;
        *(float *)((int)_this + 0x74) =
             (*(float *)((int)_this + iVar4 * 4 + 0x34) - *(float *)((int)_this + iVar4 * 4 + 0x1c)) *
             (_DAT_00784250 - fVar3 * fVar3) + *(float *)((int)_this + iVar4 * 4 + 0x1c);
        return;
      default:
        *(float *)((int)_this + 0x74) =
             (fVar3 / *(float *)((int)_this + iVar4 * 4 + 4)) *
             (*(float *)((int)_this + iVar4 * 4 + 0x34) - *(float *)((int)_this + iVar4 * 4 + 0x1c)) +
             *(float *)((int)_this + iVar4 * 4 + 0x1c);
        break;
      case 6:
        uVar6 = FUN_0075bed4();
        iVar4 = (int)uVar6;
        if (iVar4 < 1) {
          iVar4 = 0;
        }
        else if (0x7fff < iVar4) {
          iVar4 = 0x7fff;
        }
        uVar5 = FUN_00468160(2,0x7fff - iVar4,'\0');
        *(float *)((int)_this + 0x74) =
             (float)((float10)(int)uVar5 * (float10)_DAT_0078a6c8 * extraout_ST0 +
                    (float10)*(float *)((int)_this + *(int *)((int)_this + 0x70) * 4 + 0x1c));
        return;
      }
    }
    else {
      *(undefined4 *)((int)_this + 0x74) = *(undefined4 *)((int)_this + iVar4 * 4 + 0x34);
      if (*(int *)((int)_this + 0x6c) + -1 <= iVar4) {
        *(undefined1 *)((int)_this + 0x78) = 1;
        return;
      }
      iVar1 = iVar4 + 1;
      fVar2 = *(float *)((int)_this + iVar4 * 4 + 4);
      *(int *)((int)_this + 0x70) = iVar1;
      *(float *)_this = fVar3 - fVar2;
      if (*(char *)(iVar4 + 0x4d + (int)_this) != '\0') {
        *(undefined4 *)((int)_this + iVar1 * 4 + 0x1c) =
             *(undefined4 *)((int)_this + iVar1 * 4 + 0x30);
        return;
      }
    }
  }
  return;
}


