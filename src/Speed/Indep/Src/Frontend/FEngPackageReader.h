#ifndef UG2_FENG_PACKAGE_READER_H
#define UG2_FENG_PACKAGE_READER_H
#include "FEngPackage.h"

// Observed prefix of the package reader shared by 0x5554a0 and the object reader 0x555140.
// The routines called with this reader (string, list, grid, script, response, factory and
// inheritance readers) each view the same memory through their own prefix structure.
struct FEngPackageReader {
    FEPackage *package0;
    const unsigned char *data4; // package chunk: id, size, then sub-chunks
    unsigned char value8;       // nonzero: the Init script tracks are not applied
    unsigned char value9;       // nonzero: 'On' sets the object name (as in FEngStringPropertyReader)
    unsigned char valueA;       // script reader 'Sn' name flag (FEngScriptReader)
    char unidentifiedB;
    HudVisibilityObject *objectC;
    HudVisibilityObject *parent10; // last 'PA' lookup result
    HudVisibilityObject *group14;  // group (type 5) that receives the next object, else the package
    unsigned char value18;         // object was built from an inherited object
    char unidentified19[3];
    HudVisibilityObject *inherited1C; // 0x5186c0 output
    FEPackage *inheritedPackage20;    // 0x5186c0 output
    char unidentified24[0x24];
    unsigned int count48;          // 'Butn' focus object capacity
    unsigned int used4C;           // focus objects stored so far
    bool Read(); // 0x5554a0
    unsigned char ReadObject(const unsigned short *data, unsigned int size); // 0x555140, AL only
};
UG2_CHECK_OFFSET(FEngPackageReader, data4, 4);
UG2_CHECK_OFFSET(FEngPackageReader, value8, 8);
UG2_CHECK_OFFSET(FEngPackageReader, objectC, 0xc);
UG2_CHECK_OFFSET(FEngPackageReader, parent10, 0x10);
UG2_CHECK_OFFSET(FEngPackageReader, group14, 0x14);
UG2_CHECK_OFFSET(FEngPackageReader, value18, 0x18);
UG2_CHECK_OFFSET(FEngPackageReader, inherited1C, 0x1c);
UG2_CHECK_OFFSET(FEngPackageReader, inheritedPackage20, 0x20);
UG2_CHECK_OFFSET(FEngPackageReader, count48, 0x48);
UG2_CHECK_OFFSET(FEngPackageReader, used4C, 0x4c);
#endif
