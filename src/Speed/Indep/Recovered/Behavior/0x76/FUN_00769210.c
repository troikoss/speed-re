// Original UG2 PC v1.2 0x769210, 184 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
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



/* Library Function - Single Match
    _memcmp
   
   Libraries: Visual Studio 2003 Debug, Visual Studio 2003 Release */

int __cdecl FUN_00769210(void *_Buf1,void *_Buf2,size_t _Size)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  byte bVar4;
  uint uVar5;
  byte bVar6;
  uint *puVar7;
  uint *puVar8;
  bool bVar9;
  
  if (_Size != 0) {
    if ((((uint)_Buf1 | (uint)_Buf2) & 3) == 0) {
      uVar2 = _Size & 3;
      uVar5 = _Size >> 2;
      bVar9 = false;
      puVar7 = _Buf1;
      puVar8 = _Buf2;
      if (uVar5 != 0) {
        do {
          _Buf1 = puVar7;
          _Buf2 = puVar8;
          if (uVar5 == 0) break;
          uVar5 = uVar5 - 1;
          _Buf2 = puVar8 + 1;
          _Buf1 = puVar7 + 1;
          bVar9 = *puVar7 == *puVar8;
          puVar7 = _Buf1;
          puVar8 = _Buf2;
        } while (bVar9);
        if (!bVar9) {
          uVar2 = *(uint *)((int)_Buf1 + -4);
          uVar5 = *(uint *)((int)_Buf2 + -4);
          bVar9 = (byte)uVar2 < (byte)uVar5;
          if ((((byte)uVar2 == (byte)uVar5) &&
              (bVar4 = (byte)(uVar2 >> 8), bVar6 = (byte)(uVar5 >> 8), bVar9 = bVar4 < bVar6,
              bVar4 == bVar6)) &&
             (bVar4 = (byte)(uVar2 >> 0x10), bVar6 = (byte)(uVar5 >> 0x10), bVar9 = bVar4 < bVar6,
             bVar4 == bVar6)) {
            bVar9 = (byte)(uVar2 >> 0x18) < (byte)(uVar5 >> 0x18);
          }
          goto LAB_00769290;
        }
      }
      if (uVar2 != 0) {
        uVar5 = *(uint *)_Buf1;
        uVar1 = *(uint *)_Buf2;
        bVar9 = (byte)uVar5 < (byte)uVar1;
        if ((byte)uVar5 != (byte)uVar1) {
LAB_00769290:
          return (1 - (uint)bVar9) - (uint)(bVar9 != 0);
        }
        iVar3 = 0;
        if (uVar2 != 1) {
          bVar6 = (byte)(uVar5 >> 8);
          bVar4 = (byte)(uVar1 >> 8);
          bVar9 = bVar6 < bVar4;
          if (bVar6 != bVar4) goto LAB_00769290;
          iVar3 = 0;
          if (uVar2 != 2) {
            bVar9 = (uVar5 & 0xff0000) < (uVar1 & 0xff0000);
            if ((uVar5 & 0xff0000) != (uVar1 & 0xff0000)) goto LAB_00769290;
            iVar3 = uVar2 - 3;
          }
        }
        return iVar3;
      }
    }
    else {
      if ((_Size & 1) == 0) goto LAB_00769243;
      bVar9 = *(byte *)_Buf1 < *(byte *)_Buf2;
      if (*(byte *)_Buf1 != *(byte *)_Buf2) goto LAB_00769290;
      _Buf1 = (void *)((int)_Buf1 + 1);
      _Buf2 = (void *)((int)_Buf2 + 1);
      for (_Size = _Size - 1; _Size != 0; _Size = _Size - 2) {
LAB_00769243:
        bVar9 = *(byte *)_Buf1 < *(byte *)_Buf2;
        if ((*(byte *)_Buf1 != *(byte *)_Buf2) ||
           (bVar9 = *(byte *)((int)_Buf1 + 1) < *(byte *)((int)_Buf2 + 1),
           *(byte *)((int)_Buf1 + 1) != *(byte *)((int)_Buf2 + 1))) goto LAB_00769290;
        _Buf2 = (void *)((int)_Buf2 + 2);
        _Buf1 = (void *)((int)_Buf1 + 2);
      }
    }
  }
  return 0;
}


