#include "CarRenderConsume.h"
// Original 0x48b1e0. Keep this leaf separate from queue consumption so upload
// callers do not require unrelated pool/queue ownership merely to select a view.
CarRenderViewRecord *CarRenderViewByIndex(int index) { return ug2RenderViews832DE0+index; }
