#include "CarRenderTechniqueMap.h"

// Original 0x5ce070. Allocation failure is not a supported empty-list state:
// the original still writes the previous pointer at address 4 on NULL return.
CarRenderTechniqueMapNode *CarRenderTechniqueMapSentinel() {
    CarRenderTechniqueMapNode *node = static_cast<CarRenderTechniqueMapNode *>(UG2TextureMalloc(0x10));
    if (node) node->next00 = node;
    unsigned int previous = reinterpret_cast<unsigned int>(node)+4;
    if (previous) *reinterpret_cast<CarRenderTechniqueMapNode **>(previous) = node;
    return node;
}

// Original 0x5ce0f0. Re-read value each iteration; skip only a NULL destination.
void CarRenderTechniqueMapFill(CarRenderTechniqueMapNode **output,unsigned int count,
    CarRenderTechniqueMapNode *const *value,void *,unsigned int) {
    for (;count;--count,++output) if (output) *output = *value;
}

// Original 0x5ca630. Forward copy, including overlapping ranges.
CarRenderTechniqueMapNode **CarRenderTechniqueMapCopy(CarRenderTechniqueMapNode **first,
    CarRenderTechniqueMapNode **last,CarRenderTechniqueMapNode **output,void *,unsigned int) {
    for (;first!=last;++first,++output) if (output) *output = *first;
    return output;
}

// Original 0x5ca610.
void CarRenderTechniqueMapFillRange(CarRenderTechniqueMapNode **first,
    CarRenderTechniqueMapNode **last,CarRenderTechniqueMapNode *const *value) {
    for (;first!=last;++first) *first = *value;
}

// Original 0x5ce0c0.
void CarRenderTechniqueMapCopyBackward(CarRenderTechniqueMapNode ***result,
    CarRenderTechniqueMapNode **first,CarRenderTechniqueMapNode **last,
    CarRenderTechniqueMapNode **output) {
    while (first!=last) *--output = *--last;
    *result = output;
}

// Original 0x5cf190, allocator wrapper around 0x5ca630.
CarRenderTechniqueMapNode **CarRenderTechniqueMapVector::Copy(CarRenderTechniqueMapNode **first,
    CarRenderTechniqueMapNode **last,CarRenderTechniqueMapNode **output) {
    return CarRenderTechniqueMapCopy(first,last,output,this,reinterpret_cast<unsigned int>(output));
}

// Original 0x5cfbc0, allocator wrapper around 0x5ce0f0.
CarRenderTechniqueMapNode **CarRenderTechniqueMapVector::Fill(CarRenderTechniqueMapNode **output,
    unsigned int count,CarRenderTechniqueMapNode *const *value) {
    CarRenderTechniqueMapFill(output,count,value,this,reinterpret_cast<unsigned int>(value));
    return output+count;
}

// Original 0x5d0250. Normal control flow; Windows exception registration omitted.
void CarRenderTechniqueMapVector::Initialise(unsigned int count,CarRenderTechniqueMapNode *const *value) {
    CarRenderTechniqueMapNode **output = begin04;
    CarRenderTechniqueMapFill(output,count,value,this,reinterpret_cast<unsigned int>(value));
    end08 = output+count;
}

// Original 0x5d0890. Capture the fill value before any moves or allocation.
// Allocator failure/unwinding is outside the verified normal-control-flow path.
void CarRenderTechniqueMapVector::Insert(CarRenderTechniqueMapNode **position,unsigned int count,
    CarRenderTechniqueMapNode *const *value) {
    CarRenderTechniqueMapNode *saved = *value;
    unsigned int capacity = begin04 ? static_cast<unsigned int>(capacity0C-begin04) : 0;
    if (!count) return;
    unsigned int size = begin04 ? static_cast<unsigned int>(end08-begin04) : 0;
    if (0x3fffffff-size<count) { UG2TechniqueVectorTooLong(); return; }
    if (capacity<size+count) {
        if (0x3fffffff-(capacity>>1)<capacity) capacity = 0;
        else capacity += capacity>>1;
        size = begin04 ? static_cast<unsigned int>(end08-begin04) : 0;
        if (capacity<size+count) capacity = size+count;
        CarRenderTechniqueMapNode **replacement = static_cast<CarRenderTechniqueMapNode **>(UG2TextureMalloc(capacity*4));
        CarRenderTechniqueMapNode **middle = CarRenderTechniqueMapCopy(begin04,position,replacement,this,count);
        CarRenderTechniqueMapFill(middle,count,&saved,this,count);
        CarRenderTechniqueMapCopy(position,end08,middle+count,this,count);
        size = begin04 ? static_cast<unsigned int>(end08-begin04) : 0;
        if (begin04) UG2TextureFree(begin04);
        capacity0C = replacement+capacity;
        end08 = replacement+size+count;
        begin04 = replacement;
        return;
    }
    if (static_cast<unsigned int>(end08-position)<count) {
        Copy(position,end08,position+count);
        Fill(end08,count-static_cast<unsigned int>(end08-position),&saved);
        end08 += count;
    } else {
        CarRenderTechniqueMapNode **oldEnd = end08;
        CarRenderTechniqueMapNode **tail = oldEnd-count;
        end08 = Copy(tail,oldEnd,oldEnd);
        CarRenderTechniqueMapNode **result;
        CarRenderTechniqueMapCopyBackward(&result,position,tail,oldEnd);
    }
    CarRenderTechniqueMapFillRange(position,position+count,&saved);
}

// Original 0x5d0c50. Preserve capacity, clear the live range, then insert.
void CarRenderTechniqueMapVector::Assign(unsigned int count,CarRenderTechniqueMapNode *const *value) {
    CarRenderTechniqueMapNode *saved = *value;
    if (begin04!=end08) end08 = begin04;
    Insert(begin04,count,&saved);
}

// Original 0x5cf140. Unlink and free before decrementing count.
void CarRenderTechniqueMapList::Erase(CarRenderTechniqueMapNode **output,
    CarRenderTechniqueMapNode *first,CarRenderTechniqueMapNode *last) {
    while (first!=last) {
        CarRenderTechniqueMapNode *node = first;
        first = node->next00;
        if (node!=head04) {
            node->previous04->next00 = node->next00;
            node->next00->previous04 = node->previous04;
            UG2TextureFree(node);
            --count08;
        }
    }
    *output = first;
}

// Original 0x5d0bb0, RET 8. Only the first tag byte is copied; tagB is unused.
CarRenderTechniqueMap *CarRenderTechniqueMap::Construct(const unsigned char *tagA,const unsigned char *) {
    value00[0] = *tagA;
    head08 = CarRenderTechniqueMapSentinel();
    count0C = 0;
    CarRenderTechniqueMapNode *head = head08;
    begin14 = 0;
    end18 = 0;
    capacity1C = 0;
    begin14 = static_cast<CarRenderTechniqueMapNode **>(UG2TextureMalloc(0x24));
    end18 = begin14;
    capacity1C = begin14+9;
    reinterpret_cast<CarRenderTechniqueMapVector *>(&value10)->Initialise(9,&head);
    mask20 = 1;
    buckets24 = 1;
    return this;
}

// Original 0x5d0660. The bucket storage is freed before the list entries.
void CarRenderTechniqueMap::Destroy() {
    if (begin14) UG2TextureFree(begin14);
    begin14 = 0;
    end18 = 0;
    capacity1C = 0;
    CarRenderTechniqueMapNode *result;
    reinterpret_cast<CarRenderTechniqueMapList *>(value00+4)->Erase(&result,head08 ? head08->next00 : 0,head08);
    UG2TextureFree(head08);
    head08 = 0;
    count0C = 0;
}

// Original 0x5d0f20. Cached parameter handles are deliberately preserved.
void CarRenderTechniqueParameters::Reset() {
    CarRenderTechniqueMapNode *result;
    reinterpret_cast<CarRenderTechniqueMapList *>(map960.value00+4)->Erase(&result,
        map960.head08 ? map960.head08->next00 : 0,map960.head08);
    CarRenderTechniqueMapNode *head = map960.head08;
    reinterpret_cast<CarRenderTechniqueMapVector *>(&map960.value10)->Assign(9,&head);
    map960.mask20 = 1;
    map960.buckets24 = 1;
}

// Original 0x5ce090, RET 12. Pair loads follow allocation and the link stores.
CarRenderTechniqueMapNode *CarRenderTechniqueMapList::CreateNode(CarRenderTechniqueMapNode *next,
    CarRenderTechniqueMapNode *previous,const CarRenderTechniqueMapPair *pair) {
    CarRenderTechniqueMapNode *node = static_cast<CarRenderTechniqueMapNode *>(UG2TextureMalloc(0x10));
    if (node) {
        node->next00 = next;
        node->previous04 = previous;
        node->key08 = pair->hash00;
        node->value0C = pair->value04;
    }
    return node;
}

// Original 0x5d06c0, normal path and unsigned length guard. The inline error
// block constructs/throws a Windows library exception; it is a runtime seam.
void CarRenderTechniqueMapList::Grow(unsigned int count) {
    if (0x1fffffff-count08<count) { UG2TechniqueListTooLong(); return; }
    count08 += count;
}

// Original 0x5d02d0. Forward-copy the suffix, then write the result iterator.
void CarRenderTechniqueMapVector::Erase(CarRenderTechniqueMapNode ***result,
    CarRenderTechniqueMapNode **first,CarRenderTechniqueMapNode **last) {
    if (first!=last) {
        CarRenderTechniqueMapNode **oldEnd = end08;
        CarRenderTechniqueMapNode **output = first;
        for (;last!=oldEnd;++last,++output) *output = *last;
        end08 = output;
    }
    *result = first;
}

// Original 0x5d0b30. The fill argument is passed by value, including for growth.
void CarRenderTechniqueMapVector::Resize(unsigned int count,CarRenderTechniqueMapNode *value) {
    unsigned int size = begin04 ? static_cast<unsigned int>(end08-begin04) : 0;
    if (size<count) {
        size = begin04 ? static_cast<unsigned int>(end08-begin04) : 0;
        Insert(end08,count-size,&value);
        return;
    }
    if (begin04 && count<static_cast<unsigned int>(end08-begin04)) {
        CarRenderTechniqueMapNode **result;
        Erase(&result,begin04+count,end08);
    }
}

// Original 0x5d0cf0, RET 8. One linear-hash bucket may split before lookup,
// including on a duplicate. Preserve the bucket updates and list-splice order.
CarRenderTechniqueMapResult *CarRenderTechniqueMap::Insert(CarRenderTechniqueMapResult *result,
    const CarRenderTechniqueMapPair *pair) {
    unsigned int bucket = buckets24;
    if (bucket<=(count0C>>2)) {
        unsigned int size = begin14 ? static_cast<unsigned int>(end18-begin14) : 0;
        if (bucket<size-1) {
            if (mask20<bucket) mask20 = mask20*2+1;
        } else {
            size = begin14 ? static_cast<unsigned int>(end18-begin14) : 0;
            mask20 = size*2-3;
            reinterpret_cast<CarRenderTechniqueMapVector *>(&value10)->Resize(size*2-1,head08);
        }
        bucket = buckets24-(mask20>>1)-1;
        CarRenderTechniqueMapNode *node = begin14[bucket];
        if (node!=begin14[bucket+1]) {
            do {
                CarRenderTechniqueMapNode *next;
                if ((node->key08&mask20)==bucket) {
                    next = node->next00;
                } else {
                    unsigned int upper = buckets24;
                    while (bucket<upper && begin14[upper]==head08) {
                        begin14[upper] = node;
                        --upper;
                    }
                    next = node->next00;
                    if (next==head08) break;
                    unsigned int lower = bucket;
                    while (node==begin14[lower]) {
                        begin14[lower] = begin14[lower]->next00;
                        if (!lower) break;
                        --lower;
                    }
                    CarRenderTechniqueMapNode *head = head08;
                    if (node!=head08 && head!=node && head!=node->next00) {
                        CarRenderTechniqueMapNode *following = node->next00;
                        node->previous04->next00 = following;
                        following->previous04->next00 = head;
                        head->previous04->next00 = node;
                        CarRenderTechniqueMapNode *previous = head->previous04;
                        head->previous04 = following->previous04;
                        following->previous04 = node->previous04;
                        node->previous04 = previous;
                    }
                    begin14[buckets24+1] = head08;
                }
                node = next;
            } while (node!=begin14[bucket+1]);
        }
        ++buckets24;
    }
    unsigned int key = pair->hash00;
    bucket = key&mask20;
    if (buckets24<=bucket) bucket += -1-(mask20>>1);
    CarRenderTechniqueMapNode *node = begin14[bucket+1];
    if (node!=begin14[bucket]) {
        do {
            node = node->previous04;
            if (node->key08<=key) {
                if (key<=node->key08) {
                    result->node00 = node;
                    result->inserted04 = 0;
                    return result;
                }
                node = node->next00;
                break;
            }
        } while (node!=begin14[bucket]);
    }
    CarRenderTechniqueMapList *list = reinterpret_cast<CarRenderTechniqueMapList *>(value00+4);
    CarRenderTechniqueMapNode *created = list->CreateNode(node,node->previous04,pair);
    list->Grow(1);
    node->previous04 = created;
    created->previous04->next00 = created;
    CarRenderTechniqueMapNode *inserted = node->previous04;
    while (node==begin14[bucket]) {
        begin14[bucket] = inserted;
        if (!bucket) break;
        --bucket;
    }
    result->node00 = inserted;
    result->inserted04 = 1;
    return result;
}
