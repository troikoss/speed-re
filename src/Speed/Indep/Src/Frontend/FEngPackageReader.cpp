#include "FEngPackageReader.h"
#include "FEngObject.h"
#include "FEngObjectFactory.h"
#include "FEngStringReader.h"
#include "FEngListReader.h"
#include "FEngGridReader.h"
#include "FEngInheritance.h"
#include "FEngResponseReader.h"
#include "FEngResponsePool.h"
#include "FEngScriptReader.h"
#include "FEngScriptPool.h"
#include "FEngTrackEvaluate.h"
#include "FEngLookup.h"

// FNG package reader chain: package chunks 0x5554a0, object chunks 0x555140, the
// object and group clones used for inherited objects (0x554300, 0x5548a0), the list
// appends they end in (0x517e90, 0x495610, 0x518280) and the package loader 0x555860.
//
// Chunk ids are little-endian four-character tags followed by a size word; property
// ids inside an object chunk are 16-bit tags followed by a 16-bit size. The retail
// code trusts all sizes, tags and pointers, as does this recovery.

// Object virtual slots used here: 0 is the deleting destructor (flags bit 0 frees the memory) and
// 1 clones the object. Natively the vtable word is an opaque token, so these are not callable there.
struct FEngObjectVirtuals {
    virtual FEngObjectVirtuals *Delete(unsigned int flags) = 0;
    virtual HudVisibilityObject *Clone(unsigned int shallow) = 0;
};

// Original 0x517ee0, the visitor 0x555860 applies to a reused package: select the Init
// script (hash 0x1744b3), run it, and mark the object dirty. The original stores to the
// selected script's +0x10 without a null check, so a package object without an Init script
// writes through a null pointer. The store is volatile so optimising compilers keep the
// selection loop's null test and the guarded call, as in the retail code.
struct FEngInitScriptVisitor : FEngObjectVisitor {
    unsigned char Visit(HudVisibilityObject *object);
};
unsigned char FEngInitScriptVisitor::Visit(HudVisibilityObject *object) {
    FEScript *script = object->scripts4C;
    while (script && script->hash30 != 0x1744b3) script = script->next4;
    object->currentScript54 = script;
    if (script) object->InitializeScript();
    *(volatile int *)&object->currentScript54->time10 = 0; // volatile: keep the unguarded store of the original
    object->flags1C |= 0x3c00000;
    return 1;
}

// Original 0x518280. The loader's insert: walk back from the last package while its
// level is greater, then insert after the stop point or at the head.
void FEngPackageList::InsertByLevel(FEPackage *package) {
    FEPackage *cursor = lastC;
    if (cursor) {
        int level = package->value18;
        do {
            if (cursor->value18 <= level) break;
            cursor = cursor->previous8;
        } while (cursor);
    }
    if (package) {
        if (cursor) {
            FEPackage *next = cursor->next4;
            package->next4 = next;
            if (next) next->previous8 = package;
            package->previous8 = cursor;
            cursor->next4 = package;
        } else {
            FEPackage *first = first8;
            package->next4 = first;
            if (first) first->previous8 = package;
            package->previous8 = 0;
            first8 = package;
        }
        if (lastC == cursor) lastC = package;
        ++count4;
    }
}

// Original 0x495610: the object list is the FEngQueueList at package +0x44 (count +0x48,
// first +0x4c, last +0x50).
void FEPackage::AppendObject(HudVisibilityObject *object) {
    FEngQueueList *list = (FEngQueueList *)(unidentified40 + 4);
    list->Insert(list->valueC, (FEngQueueNode *)object);
}

// Original 0x517e90: the group's child list is the FEngQueueList at +0x5c; its tail
// insertion is expanded in place.
void FEngGroupObject::AppendChild(HudVisibilityObject *child) {
    HudVisibilityObject *cursor = (HudVisibilityObject *)children5C.valueC;
    if (child) {
        if (cursor) {
            HudVisibilityObject *next = cursor->next4;
            child->next4 = next;
            if (next) next->previous8 = child;
            child->previous8 = cursor;
            cursor->next4 = child;
        } else {
            HudVisibilityObject *first = (HudVisibilityObject *)children5C.first8;
            child->next4 = first;
            if (first) first->previous8 = child;
            child->previous8 = 0;
            children5C.first8 = (FEngQueueNode *)child;
        }
        if (children5C.valueC == (FEngQueueNode *)cursor) children5C.valueC = (FEngQueueNode *)child;
        ++children5C.count4;
    }
}

// Original 0x554300: copy-construct an object (unconstructed receiver, SEH frame omitted). It takes a
// new GUID; type, flags, resource words, texture and name (whose setter derives the hash) are copied;
// responses and scripts are rebuilt through their pools; the state block is a fresh zeroed block of
// the source's size, and the +0x28 word stays zero.
HudVisibilityObject *HudVisibilityObject::CloneConstruct(const HudVisibilityObject *source, unsigned int shallow) {
    next4 = (HudVisibilityObject *)0xabadcafe;
    previous8 = (HudVisibilityObject *)0xabadcafe;
    *(unsigned int *)unidentified0 = 0x79b8e0;
    hash10 = 0;
    *(unsigned int *)unidentified14 = 0;
    flags1C = 0;
    texture24 = 0;
    value28 = 0;
    data2C = 0;
    *(unsigned int *)(unidentified30 + 4) = 0x79a9ec;
    responses3C = 0;
    *(unsigned int *)unidentified40 = 0;
    *(unsigned int *)(unidentified30 + 8) = 0;
    *(unsigned int *)(unidentified40 + 4) = 0x79a9ec;
    scripts4C = 0;
    *(unsigned int *)unidentified50 = 0;
    *(unsigned int *)(unidentified40 + 8) = 0;
    unsigned int guid = FEngNextObjectGuid;
    FEngNextObjectGuid = guid + 1;
    guidC = guid;
    unsigned int stride = *(const unsigned int *)(source->unidentified30);
    FEngObjectStateManager.ReleaseState(data2C);
    data2C = 0;
    void *state = FEngObjectStateManager.AcquireState(stride);
    *(unsigned int *)unidentified30 = stride;
    data2C = (float *)state;
    // Zero the new state block: whole words, then the remaining bytes.
    unsigned int *words = (unsigned int *)state;
    for (unsigned int count = stride >> 2; count; --count) *words++ = 0;
    unsigned char *bytes = (unsigned char *)words;
    for (unsigned int count = stride & 3; count; --count) *bytes++ = 0;
    type18 = source->type18;
    flags1C = source->flags1C;
    *(unsigned short *)unidentified20 = *(const unsigned short *)source->unidentified20;
    resource22 = source->resource22;
    texture24 = source->texture24;
    SetName(source->name14);

    FEngQueueList *responses = (FEngQueueList *)((char *)this + 0x34);
    for (FEResponse *original = source->responses3C; original; original = original->next4) {
        FEResponse *response = FEngResponseHeaderPool.Acquire();
        response->next4 = (FEResponse *)0xabadcafe;
        response->previous8 = (FEResponse *)0xabadcafe;
        *(unsigned int *)response->unidentified0 = 0x79aa38;
        response->messageC = 0;
        response->commandCount10 = 0;
        response->commands14 = 0;
        unsigned int count = original->commandCount10;
        response->ResizeCommands(count);
        response->messageC = original->messageC;
        if (count > 0) {
            unsigned int offset = 0;
            unsigned int remaining = count;
            do {
                ((FEResponseCommand *)((char *)response->commands14 + offset))->Assign((FEResponseCommand *)((char *)original->commands14 + offset));
                offset += 0xc;
                --remaining;
            } while (remaining);
        }
        FEngQueueNode *cursor = responses->valueC;
        FEngQueueNode *node = (FEngQueueNode *)response;
        if (cursor) {
            FEngQueueNode *next = cursor->next4;
            node->next4 = next;
            if (next) next->previous8 = node;
            node->previous8 = cursor;
            cursor->next4 = node;
        } else {
            FEngQueueNode *first = responses->first8;
            node->next4 = first;
            if (first) first->previous8 = node;
            node->previous8 = 0;
            responses->first8 = node;
        }
        if (responses->valueC == cursor) responses->valueC = node;
        ++responses->count4;
    }

    FEngQueueList *list = (FEngQueueList *)((char *)this + 0x44);
    for (FEScript *original = source->scripts4C; original; original = original->next4) {
        FEScript *header = FEngScriptHeaderPool.Acquire();
        header->name2C = 0;
        header->hash30 = 0;
        header->lengthC = 0;
        header->flags14 = 0;
        header->chain18 = 0;
        header->trackCount1C = 0;
        header->tracks20 = 0;
        FEScript *script = header->CopyConstruct(original, (unsigned char)shallow);
        FEngQueueNode *cursor = list->valueC;
        if (script) {
            FEngQueueNode *node = (FEngQueueNode *)script;
            if (cursor) {
                FEngQueueNode *next = cursor->next4;
                node->next4 = next;
                if (next) next->previous8 = node;
                node->previous8 = cursor;
                cursor->next4 = node;
            } else {
                FEngQueueNode *first = list->first8;
                node->next4 = first;
                if (first) first->previous8 = node;
                node->previous8 = 0;
                list->first8 = node;
            }
            if (list->valueC == cursor) list->valueC = node;
            ++list->count4;
        }
    }

    // Select the source's current script by hash in the copy, and run it.
    FEScript *found = scripts4C;
    if (found) {
        unsigned int hash = source->currentScript54->hash30;
        while (found && found->hash30 != hash) found = found->next4;
    }
    currentScript54 = found;
    if (found) InitializeScript();

    // Relink each script's chain pointer to the copy with the same hash as the
    // source script's chain target.
    FEScript *sourceScript = source->scripts4C;
    FEScript *copy = scripts4C;
    while (sourceScript) {
        if (sourceScript->chain18) {
            FEScript *target = scripts4C;
            if (target) {
                unsigned int hash = sourceScript->chain18->hash30;
                while (target && target->hash30 != hash) target = target->next4;
            }
            copy->chain18 = target;
        }
        sourceScript = sourceScript->next4;
        copy = copy->next4;
    }
    return this;
}

// Original 0x5548a0: group copy-construct. The base copy comes first; children are
// cloned through virtual slot +4 only when copyChildren is set.
FEngGroupObject *FEngGroupObject::CloneConstruct(const FEngGroupObject *source, unsigned char copyChildren, unsigned int shallow) {
    ((HudVisibilityObject *)this)->CloneConstruct((const HudVisibilityObject *)source, shallow);
    *(unsigned int *)unidentified0 = 0x79b8e8;
    children5C.value0 = 0x79a9ec;
    children5C.first8 = 0;
    children5C.valueC = 0;
    children5C.count4 = 0;
    if (copyChildren) {
        for (HudVisibilityObject *child = (HudVisibilityObject *)source->children5C.first8; child; child = child->next4) {
            HudVisibilityObject *copy = ((FEngObjectVirtuals *)child)->Clone(shallow);
            HudVisibilityObject *cursor = (HudVisibilityObject *)children5C.valueC;
            if (copy) {
                if (cursor) {
                    HudVisibilityObject *next = cursor->next4;
                    copy->next4 = next;
                    if (next) next->previous8 = copy;
                    copy->previous8 = cursor;
                    cursor->next4 = copy;
                } else {
                    HudVisibilityObject *first = (HudVisibilityObject *)children5C.first8;
                    copy->next4 = first;
                    if (first) first->previous8 = copy;
                    copy->previous8 = 0;
                    children5C.first8 = (FEngQueueNode *)copy;
                }
                if (children5C.valueC == (FEngQueueNode *)cursor) children5C.valueC = (FEngQueueNode *)copy;
                ++children5C.count4;
            }
        }
    }
    return this;
}

// Original 0x555140: one object chunk. Returns 0 only when an inherited object
// cannot be resolved; the half-built object is then deleted and objectC cleared.
unsigned char FEngPackageReader::ReadObject(const unsigned short *data, unsigned int size) {
    const unsigned short *end = (const unsigned short *)((const char *)data + size);
    const unsigned short *property = data;
    value18 = 0;
    while (property < end) {
        const unsigned int *payload = (const unsigned int *)(property + 2);
        switch (property[0]) {
        case 0x504f: // OP
            objectC->guidC = payload[0];
            objectC->hash10 = payload[1];
            objectC->flags1C = payload[2];
            objectC->resource22 = (short)payload[3];
            if (objectC->flags1C & 0x100000) {
                if (!((FEngInheritanceContext *)this)->Resolve(objectC->guidC, &inherited1C, &inheritedPackage20)) {
                    if (objectC) ((FEngObjectVirtuals *)objectC)->Delete(1);
                    objectC = 0;
                    return 0;
                }
                value18 = 1;
                objectC->flags1C |= inherited1C->flags1C;
                if (objectC->resource22 == (short)0xffff) {
                    objectC->texture24 = inherited1C->texture24;
                    objectC->value28 = inherited1C->value28;
                }
                HudVisibilityObject *clone;
                if (inherited1C->type18 != 5) {
                    clone = ((FEngObjectVirtuals *)inherited1C)->Clone(1);
                } else {
                    FEngGroupObject *group = (FEngGroupObject *)FEngAllocateQueueNode(0x6c);
                    if (group) clone = (HudVisibilityObject *)group->CloneConstruct((FEngGroupObject *)inherited1C, 0, 1);
                    else clone = 0;
                }
                clone->guidC = objectC->guidC;
                clone->hash10 = objectC->hash10;
                clone->flags1C = objectC->flags1C;
                clone->resource22 = objectC->resource22;
                clone->texture24 = objectC->texture24;
                clone->value28 = objectC->value28;
                if (objectC) ((FEngObjectVirtuals *)objectC)->Delete(1);
                objectC = clone;
                for (FEScript *script = clone->scripts4C; script; script = script->next4) {
                    if (script->chain18) script->chain18 = (FEScript *)script->chain18->hash30;
                }
            }
            if (objectC->flags1C & 0x10000000) {
                package0->focusObjects90[used4C] = objectC;
                ++used4C;
            }
            break;
        case 0x4150: // PA
            if (!parent10 || parent10->guidC != payload[0]) parent10 = package0->FindObjectByGuid(payload[0]);
            group14 = parent10;
            break;
        case 0x4153: { // SA
            unsigned int count = property[1] >> 2;
            for (unsigned int i = 0; i < count; ++i) ((unsigned int *)objectC->data2C)[i] = payload[i];
            break;
        }
        case 0x684f: // Oh
            objectC->hash10 = payload[0];
            break;
        case 0x6e4f: // On
            if (value9) objectC->SetName((const char *)payload);
            break;
        case 0x744f: // Ot
            objectC = ((FEngObjectFactoryContext *)this)->Create((int)payload[0]);
            break;
        default:
            if (objectC) {
                switch (objectC->type18) {
                case 1:
                case 9:
                case 10:
                    if (property[0] == 0x6649) ((FEngLeafObject *)objectC)->value5C = payload[0];
                    break;
                case 2:
                    ((FEngStringPropertyReader *)this)->ReadProperty(property);
                    break;
                case 4:
                    ((FEngListPropertyReader *)this)->ReadProperty(property);
                    break;
                case 6:
                    ((FEngGridPropertyReader *)this)->ReadProperty(property);
                    break;
                }
            }
            break;
        }
        property = (const unsigned short *)((const char *)property + property[1] + 4);
    }
    return 1;
}

// Original 0x5554a0: read the package chunk 0xcc6a624f, whose 'Butn' chunk sizes the
// focus object array and whose object-list chunks hold the objects, responses and
// scripts. After each object list the finished object gets its type setup, script
// links, Init script and track states, and is appended to its group or the package.
// Note that every chunk after the first object list is read as an object list
// regardless of its id.
bool FEngPackageReader::Read() {
    const unsigned char *chunk = data4 + 8;
    const unsigned char *chunkEnd = data4 + *(const unsigned int *)(data4 + 4) + 8;
    while (*(const unsigned int *)chunk != 0xcc6a624f && chunk != chunkEnd)
        chunk = chunk + *(const unsigned int *)(chunk + 4) + 8;
    if (*(const unsigned int *)chunk != 0xcc6a624f) return false;
    const unsigned char *cursor = chunk + 8;
    const unsigned char *end = chunk + *(const unsigned int *)(chunk + 4) + 8;
    if (!cursor || !end) return false;
    while (*(const unsigned int *)cursor != 0xea624f46) {
        if (cursor >= end) return true;
        if (*(const unsigned int *)cursor == 0x6e747542) { // Butn
            FEPackage *package = package0;
            unsigned int count = *(const unsigned int *)(cursor + 8);
            count48 = count;
            FEngFreeQueueNode(package->focusObjects90);
            package->focusObjects90 = 0;
            if (count) package->focusObjects90 = (HudVisibilityObject **)FEngAllocateQueueNode(count * 4);
            package->focusCount94 = count;
        }
        cursor = cursor + *(const unsigned int *)(cursor + 4) + 8;
    }
    if (cursor >= end) return true;
    do {
        const unsigned char *list = cursor;
        const unsigned char *child = list + 8;
        const unsigned char *listEnd = list + *(const unsigned int *)(list + 4) + 8;
        objectC = 0;
        group14 = 0;
        while (child < listEnd) {
            unsigned int id = *(const unsigned int *)child;
            if (id == 0x446a624f) {
                if (!ReadObject((const unsigned short *)(child + 8), *(const unsigned int *)(child + 4))) return false;
            } else if (id == 0x5267734d) {
                ((FEngResponseReader *)this)->Read(child + 8, *(const unsigned int *)(child + 4), 0);
            } else if (id == 0x70726353) {
                ((FEngScriptReader *)this)->Read((const unsigned short *)(child + 8), *(const unsigned int *)(child + 4));
            }
            child = child + *(const unsigned int *)(child + 4) + 8;
        }
        if (objectC) {
            if (objectC->type18 == 4) ((FEListObject *)objectC)->CalculateOffsets();
            else if (objectC->type18 == 6) ((FEGridObject *)objectC)->InitialFill();
            for (FEScript *script = objectC->scripts4C; script; script = script->next4) {
                unsigned int hash = (unsigned int)script->chain18;
                if (hash) {
                    FEScript *target = objectC->scripts4C;
                    while (target && target->hash30 != hash) target = target->next4;
                    script->chain18 = target;
                }
            }
            FEScript *init = objectC->scripts4C;
            while (init && init->hash30 != 0x1744b3) init = init->next4;
            objectC->currentScript54 = init;
            if (init) objectC->InitializeScript();
            *(volatile int *)&init->time10 = 0; // unguarded in the original: a package object without Init stores at +0x10
            if (!value8) {
                for (unsigned char track = 0; track < (unsigned int)init->trackCount1C; ++track)
                    FEngApplyScriptTrack(init, track, 0, objectC);
            }
            if (group14) ((FEngGroupObject *)group14)->AppendChild(objectC);
            else package0->AppendObject(objectC);
        }
        cursor = cursor + *(const unsigned int *)(cursor + 4) + 8;
    } while (cursor < end);
    return true;
}

// Original 0x555860: find the package by name among the loaded list (+0xec), retrying
// with the text after the last '/' or '\\'; otherwise ask the loader for its data and
// build it with 0x555800. A cached package is reinitialised and unlinked from the list
// it came from. Either way it is stamped with the level and input mask, announced to
// the loader and inserted into the active list (+0xdc) by level.
FEPackage *FEngQueue::Call555860(const char *name, unsigned char level, unsigned int mask) {
    FEPackage *package = packagesEC.FindByName(name, packagesEC.first8);
    if (!package) {
        // Retry with the text after the last '/' or '\\'. The scan keeps a signed
        // byte length and, when no separator exists, leaves the start one byte before
        // the name.
        const char *end = name;
        while (*end++) {}
        signed char length = (signed char)(end - (name + 1));
        const char *start = name + length - 1;
        char c = *start;
        while (c != '/' && c != '\\' && length > 0) {
            c = *--start;
            --length;
        }
        if (length != 0) ++start;
        package = packagesEC.FindByName(start, packagesEC.first8);
    }
    if (package) {
        FEngInitScriptVisitor visitor;
        package->value15 = 1;
        package->VisitObjects(&visitor);
        ((FEngQueueList *)&packagesEC)->Remove((FEngQueueNode *)package);
    } else {
        void *block;
        unsigned char freeBlock;
        void *data = value10C->LoadPackageData(name, &block, &freeBlock);
        if (!data) return 0;
        package = Call555800(data, 0);
        if (freeBlock) FEngFreeQueueNode(block);
        if (!package) return 0;
    }
    package->inputMask1C = mask;
    package->inputMask20 = mask;
    package->inputMask24 = mask;
    package->inputMask28 = mask;
    package->value18 = level;
    package->value14 = scripts0;
    if (value10C) value10C->PackageLoaded(package);
    packagesDC.InsertByLevel(package);
    return package;
}
