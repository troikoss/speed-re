// Original UG2 PC v1.2 0x73e260, 88 bytes. BEHAVIOUR-VERIFIED ONLY (25 clean trials, 15 both-fault):
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



void FUN_0073e260(undefined4 param_1,int param_2,int param_3)

{
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if ((*(int *)(param_2 + 8) == 0x706c6179) && (*(int *)(param_3 + 0x28) == 3)) {
    local_4 = *(undefined4 *)(param_2 + 0x10);
    local_10 = *(undefined4 *)(param_3 + 4);
    local_14 = 0;
    local_8 = 0;
    local_c = 0x706c6179;
    (**(code **)(param_3 + 0x20))(param_3,&local_14,*(undefined4 *)(param_3 + 0x24));
    *(undefined4 *)(param_3 + 0x28) = 4;
  }
  return;
}


