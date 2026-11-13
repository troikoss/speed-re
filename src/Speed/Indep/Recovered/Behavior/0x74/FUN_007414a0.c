// Original UG2 PC v1.2 0x7414a0, 200 bytes. BEHAVIOUR-VERIFIED ONLY (19 clean trials, 21 both-fault):
// not byte-identical to speed2.exe; equivalent to the original on random inputs under
// differential emulation (tools/diff_test.py). Decompiler output with an inferred prelude.
typedef int dataString;
typedef int keyToSearch;

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

#define DAT_007d2498 (*(undefined4 *)0x007d2498)

char * FUN_007414a0(char *dataString,char *keyToSearch)

{
  char cVar1;
  byte *pbVar2;
  char *pcVar3;
  byte *pbVar4;
  char *pcVar5;
  byte *pbVar6;
  
  if ((((dataString == (char *)0x0) || (cVar1 = *dataString, cVar1 == '\0')) ||
      (keyToSearch == (char *)0x0)) || (*keyToSearch == '\0')) {
    return (char *)0x0;
  }
  pcVar5 = keyToSearch;
  do {
    pcVar3 = pcVar5;
    pcVar5 = pcVar3 + 1;
  } while (pcVar3[1] != '\0');
  pbVar6 = (byte *)(dataString + -1);
  pcVar5 = dataString;
  do {
    if ((cVar1 == '=') || (cVar1 == ':')) {
      if ((pbVar6[2] < 0x20) && (*pbVar6 < 0x21)) {
        return (char *)0x0;
      }
      if (dataString <= pbVar6) {
        pbVar4 = (byte *)(pcVar3 + (int)(pbVar6 + (1 - (int)pcVar5)));
        pbVar2 = pbVar6;
        do {
          if ((pbVar4 < keyToSearch) || ((&DAT_007d2498)[*pbVar2] != (&DAT_007d2498)[*pbVar4]))
          break;
          if ((pbVar4 == (byte *)keyToSearch) &&
             ((pbVar2 == (byte *)dataString || (pbVar2[-1] < 0x21)))) {
            if (pcVar5[1] == ' ') {
              return pcVar5 + 2;
            }
            return pcVar5 + 1;
          }
          pbVar2 = pbVar2 + -1;
          pbVar4 = pbVar4 + -1;
        } while (dataString <= pbVar2);
      }
    }
    cVar1 = pcVar5[1];
    pcVar5 = pcVar5 + 1;
    pbVar6 = pbVar6 + 1;
    if (cVar1 == '\0') {
      return (char *)0x0;
    }
  } while( true );
}


