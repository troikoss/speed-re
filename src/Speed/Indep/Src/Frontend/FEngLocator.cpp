#include "FEngPackage.h"

extern FEngArchive fengArchive8637B0;

// Original 0x52cef0: mode selects live collections or the archive-node boundary.
FEPackage *FngFindPackage(const char *name) {
    if (!fengValue8384D0 && fengManager8384C4) {
        FEngQueue *queue = fengManager8384C4->queue8;
        if (!queue) return 0;
        FEPackage *package = queue->packagesDC.FindByName(name, queue->packagesDC.first8);
        if (package) return package;
        return queue->packagesEC.FindByName(name, queue->packagesEC.first8);
    }
    FEngPackageNode *node = fengArchive8637B0.FindNode(name);
    return node ? node->package18 : 0;
}
