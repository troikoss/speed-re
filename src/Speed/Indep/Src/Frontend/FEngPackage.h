#ifndef UG2_FENG_PACKAGE_H
#define UG2_FENG_PACKAGE_H

#include "../Hud/HudVisibility.h"

struct FEPackage;
struct FEngObjectVisitor;
struct FEResponse;
struct FEngSavedFocus { unsigned int key0, value4; };
struct FEngPackageList {
    char unidentified0[8];
    FEPackage *first8;
    FEPackage *FindByName(const char *name, FEPackage *first); // 0x519e50
};
struct FEngQueue {
    char unidentified0;
    unsigned char value1;
    char unidentified2[0xda];
    FEngPackageList packagesDC;
    char unidentifiedE8[4];
    FEngPackageList packagesEC;
    char unidentifiedF8[0x4050];
    unsigned int value4148;
    char value414C[16][0x100];
    int value514C;
    FEngSavedFocus value5150[32];
    void StoreFocus(unsigned int key, unsigned int value); // 0x50b300
    unsigned int LoadFocus(unsigned int key); // 0x50b360
    void ClearPackageStack(); // 0x50b390
    void ExecuteResponse(FEResponse *response, HudVisibilityObject *object, FEPackage *package, unsigned int mask); // 0x52c7c0
    void Call51AF50(FEPackage *package, int value, unsigned int mask);
    void Call51AB20(const char *name, FEPackage *package);
    void Call51AC10(const char *name, FEPackage *package, unsigned int mask);
    void Call51ADB0(FEPackage *package);
    void Call51B070(const char *name);
    void Post(unsigned int message, void *sender, struct FEPackage *package,
              int target, int value10); // 0x51aa20, thiscall
    HudVisibilityObject *FindObject(unsigned int hash, const char *package); // 0x52c570
};
UG2_CHECK_OFFSET(FEngQueue, value1, 1);
UG2_CHECK_OFFSET(FEngQueue, value4148, 0x4148);
UG2_CHECK_OFFSET(FEngQueue, value414C, 0x414c);
UG2_CHECK_OFFSET(FEngQueue, value514C, 0x514c);
UG2_CHECK_OFFSET(FEngQueue, value5150, 0x5150);
UG2_CHECK_SIZE(FEngSavedFocus, 8);
UG2_CHECK_OFFSET(FEngQueue, packagesDC, 0xdc);
UG2_CHECK_OFFSET(FEngQueue, packagesEC, 0xec);
UG2_CHECK_OFFSET(FEngPackageList, first8, 8);

// Observed FEPackage prefix; unidentified fields retain their original offsets.
struct FEPackage {
    char unidentified0[4];
    FEPackage *next4;
    char unidentified8[4];
    const char *nameC;
    unsigned int nameHash10;
    char unidentified14[8];
    unsigned int inputMask1C;
    unsigned int inputMask20;
    unsigned int inputMask24;
    unsigned int inputMask28;
    char unidentified2C[0x20];
    HudVisibilityObject *firstObject4C;
    char unidentified50[0x3c];
    HudVisibilityObject *focus8C;
    HudVisibilityObject **focusObjects90;
    unsigned int focusCount94;
    void SetFocus(FEngQueue *queue, HudVisibilityObject *object, unsigned char notify); // 0x52b3a0
    bool VisitChildren(HudVisibilityObject *group, FEngObjectVisitor *visitor); // 0x504830
    bool VisitObjects(FEngObjectVisitor *visitor); // 0x504880
    HudVisibilityObject *FindObject(unsigned int hash); // 0x517f80
    HudVisibilityObject *FindObjectByGuid(unsigned int guid); // 0x517fe0
};
UG2_CHECK_OFFSET(FEPackage, nameC, 0xc);
UG2_CHECK_OFFSET(FEPackage, firstObject4C, 0x4c);
UG2_CHECK_OFFSET(FEPackage, inputMask1C, 0x1c);
UG2_CHECK_OFFSET(FEPackage, inputMask28, 0x28);
UG2_CHECK_OFFSET(FEPackage, focus8C, 0x8c);
UG2_CHECK_OFFSET(FEPackage, focusObjects90, 0x90);
UG2_CHECK_OFFSET(FEPackage, focusCount94, 0x94);
UG2_CHECK_SIZE(FEPackage, 0x98);

struct FEngArchiveLink {
    FEngArchiveLink *next0;
    FEngArchiveLink *previous4;
};
struct FEngPackageNode {
    char unidentified0[4];
    FEngArchiveLink links4;
    unsigned int *valueC;
    unsigned int *value10;
    char unidentified14[4];
    FEPackage *package18;
};
struct FEngArchive {
    char unidentified0[4];
    FEngArchiveLink root4;
    FEngPackageNode *FindNode(const char *name); // 0x51bb70
};
UG2_CHECK_OFFSET(FEngPackageNode, links4, 4);
UG2_CHECK_OFFSET(FEngPackageNode, valueC, 0xc);
UG2_CHECK_OFFSET(FEngPackageNode, value10, 0x10);
UG2_CHECK_OFFSET(FEngArchive, root4, 4);
struct FEngManager {
    char unidentified0[8];
    FEngQueue *queue8;
    void SetFocusByHash(const char *name, unsigned int hash); // 0x52ce40
};
UG2_CHECK_OFFSET(FEngPackageNode, package18, 0x18);
UG2_CHECK_OFFSET(FEngManager, queue8, 8);

// Persisted selection belongs to the static FNG record, not the live FEPackage.
struct FngRecord {
    char unidentified0[0x10];
    int value10;
};
extern FEngManager *fengManager8384C4;
extern unsigned int fengValue8384D0;
extern FEngArchive fengArchive8637B0;
void __stdcall FngSetFocusEnabled(const char *name, unsigned int hash, unsigned char enabled); // 0x51cc60
FEPackage *FngFindPackage(const char *name); // 0x52cef0
FngRecord *FngFindRecord(unsigned int hash); // 0x50b840

#endif
