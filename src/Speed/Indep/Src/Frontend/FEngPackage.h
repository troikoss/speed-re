#ifndef UG2_FENG_PACKAGE_H
#define UG2_FENG_PACKAGE_H

#include "../Hud/HudVisibility.h"
#include "FEngQueueNodes.h"
#include "FEngTrackLookup.h"

struct FEPackage;
struct FEngObjectVisitor;
struct FEResponse;
struct FEngInheritanceRecord { unsigned int key0, packageHash4, guid8; };
struct FEngSavedFocus { unsigned int key0, value4; };
struct FEngPackageList {
    unsigned int value0;
    unsigned int count4;
    FEPackage *first8;
    FEPackage *lastC;
    FEPackage *FindByName(const char *name, FEPackage *first); // 0x519e50
    void InsertByLevel(FEPackage *package); // 0x518280, after the last package whose +0x18 level is not greater
};
struct FEngMessageAssociation {
    unsigned int message0, capacity4, count8;
    HudVisibilityObject **objectsC;
};
// One pad of the queue's input array: buttons held this and the previous frame,
// and per-button hold time in milliseconds.
struct FEngPadState {
    unsigned int previous0;
    unsigned int current4;
    unsigned int hold8[32];
    void Latch(unsigned int buttons, int delta); // 0x504fa0
};
struct FEngMousePacket { short x0, y2, z4, buttons6; };
struct FEngMouseState {
    int x0, y4, z8;
    unsigned short previousButtonsC, buttonsE;
    int hold10[3];
    char unidentified1C;
    unsigned char moved1D;
    void Latch(FEngMousePacket *packet, int delta); // 0x50b0c0
};
struct FEngQueue {
    unsigned char scripts0;   // nonzero: stepped frames with message and operation drains
    unsigned char value1;     // nonzero: pad and mouse input are read each frame
    unsigned char mouse2;     // nonzero: mouse input is read and dispatched
    char unidentified3[5];
    FEngPadState *pads8;
    FEngMouseState mouseC;
    int value2C;
    int value30;
    unsigned int consumed34;  // bits of value38 entries to subtract from the pad hold times
    unsigned int value38[19];
    char unidentified84[0x50];
    unsigned int padCountD4;
    char unidentifiedD8[4];
    FEngPackageList packagesDC;
    FEngPackageList packagesEC;
    char unidentifiedFC[8];
    FEPackage *first104; // Separate package chain used by 0x50b240.
    char unidentified108[4];
    FEngQueueObserver *value10C;
    char unidentified110[0x4004];
    FEngQueueList messages4114;
    FEngQueueList operations4124;
    FEngTypeRegistry types4134;
    unsigned int value4148;
    char value414C[16][0x100];
    int value514C;
    FEngSavedFocus value5150[32];
    unsigned char value5250;  // nonzero clears mask 0x7f7c8c for the next script step
    unsigned char value5251;
    void Update(int delta, int unlimited); // 0x558c00
    void DispatchPad(FEPackage *package); // 0x54d4b0, not recovered
    void DispatchMouse(FEPackage *package); // 0x54e020, not recovered
    void DeliverMessages(); // 0x541fc0
    void DeliverObject(HudVisibilityObject *object, FEPackage *package, unsigned int message, unsigned int mask); // 0x5367e0
    void ConsumeOperations(); // 0x5559c0
    void Call5362E0(FEPackage *package);
    FEPackage *Call555860(const char *name, unsigned char level, unsigned int mask); // 0x555860
    FEPackage *Call555800(void *data, int value); // 0x555800 -> 0x555720 package load driver, not recovered
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
    FEPackage *FindByEncodedNameHash(unsigned int hash); // 0x50b240
};
UG2_CHECK_OFFSET(FEngQueue, value1, 1);
UG2_CHECK_OFFSET(FEngQueue, pads8, 8);
UG2_CHECK_OFFSET(FEngQueue, mouseC, 0xc);
UG2_CHECK_OFFSET(FEngQueue, value2C, 0x2c);
UG2_CHECK_OFFSET(FEngQueue, consumed34, 0x34);
UG2_CHECK_OFFSET(FEngQueue, value38, 0x38);
UG2_CHECK_OFFSET(FEngQueue, padCountD4, 0xd4);
UG2_CHECK_OFFSET(FEngQueue, value5250, 0x5250);
UG2_CHECK_SIZE(FEngPadState, 0x88);
UG2_CHECK_OFFSET(FEngMouseState, hold10, 0x10);
UG2_CHECK_OFFSET(FEngMouseState, moved1D, 0x1d);
UG2_CHECK_OFFSET(FEngQueue, value10C, 0x10c);
UG2_CHECK_OFFSET(FEngQueue, first104, 0x104);
UG2_CHECK_OFFSET(FEngQueue, messages4114, 0x4114);
UG2_CHECK_OFFSET(FEngQueue, operations4124, 0x4124);
UG2_CHECK_OFFSET(FEngQueue, value5251, 0x5251);
UG2_CHECK_OFFSET(FEngQueue, types4134, 0x4134);
UG2_CHECK_OFFSET(FEngQueue, value4148, 0x4148);
UG2_CHECK_OFFSET(FEngQueue, value414C, 0x414c);
UG2_CHECK_OFFSET(FEngQueue, value514C, 0x514c);
UG2_CHECK_OFFSET(FEngQueue, value5150, 0x5150);
UG2_CHECK_SIZE(FEngSavedFocus, 8);
UG2_CHECK_OFFSET(FEngQueue, packagesDC, 0xdc);
UG2_CHECK_OFFSET(FEngQueue, packagesEC, 0xec);
UG2_CHECK_OFFSET(FEngPackageList, first8, 8);
UG2_CHECK_OFFSET(FEngPackageList, lastC, 0xc);
UG2_CHECK_SIZE(FEngPackageList, 0x10);
UG2_CHECK_OFFSET(FEngQueue, packagesEC, 0xec);
UG2_CHECK_OFFSET(FEngQueue, unidentifiedFC, 0xfc);

// Observed FEPackage prefix; unidentified fields retain their original offsets.
struct FEPackage {
    char unidentified0[4];
    FEPackage *next4;
    FEPackage *previous8;
    const char *nameC;
    unsigned int nameHash10;
    unsigned char value14;   // nonzero: script updates fire events and maintain dirty flags (0x54ce60)
    unsigned char value15;   // set by 0x555860 when a cached package is reused
    char unidentified16[2];
    int value18;             // load level; 0x518280 orders the active list by it
    unsigned int inputMask1C;
    unsigned int inputMask20;
    unsigned int inputMask24;
    unsigned int inputMask28;
    char unidentified2C[4];
    FEngQueue *queue30;      // event delivery target of 0x534c70
    int delta34;             // script clock advance in milliseconds (0x54ce60)
    const unsigned char *value38; // 0x50b240 hashes bytes at this pointer +2.
    FEPackage *parent3C;
    char unidentified40[0xc];
    HudVisibilityObject *firstObject4C;
    char unidentified50[8];
    unsigned int responseCount58;
    FEResponse *responses5C;
    FEResponse *responseCursor60;
    char unidentified64[8];
    unsigned int associationCount6C;
    FEngMessageAssociation *associations70;
    char unidentified74[0x10];
    unsigned int inheritanceCount84;
    FEngInheritanceRecord *inheritance88;
    HudVisibilityObject *focus8C;
    HudVisibilityObject **focusObjects90;
    unsigned int focusCount94;
    void AppendResponse(FEResponse *response); // 0x512120, inline list insertion
    void SetFocus(FEngQueue *queue, HudVisibilityObject *object, unsigned char notify); // 0x52b3a0
    bool VisitChildren(HudVisibilityObject *group, FEngObjectVisitor *visitor); // 0x504830
    bool VisitObjects(FEngObjectVisitor *visitor); // 0x504880
    HudVisibilityObject *FindObject(unsigned int hash); // 0x517f80
    HudVisibilityObject *FindObjectByGuid(unsigned int guid); // 0x517fe0
    FEngInheritanceRecord *FindInheritance(unsigned int key); // 0x504960
    void ApplyScript(HudVisibilityObject *object, FEScript *script); // 0x541db0
    void FireScriptEvents(FEngQueue *queue, HudVisibilityObject *object, FEScript *script, int from, int to); // 0x534c70
    void UpdateObject(HudVisibilityObject *object, int delta); // 0x54ce60
    void UpdateChildren(HudVisibilityObject *group, int delta); // 0x504910
    void AppendObject(HudVisibilityObject *object); // 0x495610, tail insert into the list at +0x44
};
UG2_CHECK_SIZE(FEngInheritanceRecord, 12);
UG2_CHECK_OFFSET(FEPackage, previous8, 8);
UG2_CHECK_OFFSET(FEPackage, value14, 0x14);
UG2_CHECK_OFFSET(FEPackage, value15, 0x15);
UG2_CHECK_OFFSET(FEPackage, queue30, 0x30);
UG2_CHECK_OFFSET(FEPackage, delta34, 0x34);
UG2_CHECK_OFFSET(FEPackage, value38, 0x38);
UG2_CHECK_OFFSET(FEPackage, inheritanceCount84, 0x84);
UG2_CHECK_OFFSET(FEPackage, inheritance88, 0x88);
UG2_CHECK_OFFSET(FEPackage, responseCount58, 0x58);
UG2_CHECK_OFFSET(FEPackage, responses5C, 0x5c);
UG2_CHECK_OFFSET(FEPackage, responseCursor60, 0x60);
UG2_CHECK_OFFSET(FEPackage, associationCount6C, 0x6c);
UG2_CHECK_OFFSET(FEPackage, associations70, 0x70);
UG2_CHECK_SIZE(FEngMessageAssociation, 0x10);
UG2_CHECK_OFFSET(FEPackage, value18, 0x18);
UG2_CHECK_OFFSET(FEPackage, parent3C, 0x3c);
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
struct FEngRegisteredPackageLink {
    FEngRegisteredPackageLink *next0;
    FEngRegisteredPackageLink *previous4;
    const char *name8; // Used on non-sentinel records only.
};
struct FEngManager {
    char unidentified0[8];
    FEngQueue *queue8;
    char unidentifiedC[4];
    FEngRegisteredPackageLink names10; // Circular sentinel: only its two link words are observed.
    void SetFocusByHash(const char *name, unsigned int hash); // 0x52ce40
};
UG2_CHECK_OFFSET(FEngPackageNode, package18, 0x18);
UG2_CHECK_OFFSET(FEngManager, queue8, 8);
UG2_CHECK_OFFSET(FEngManager, names10, 0x10);
UG2_CHECK_OFFSET(FEngRegisteredPackageLink, name8, 8);

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
