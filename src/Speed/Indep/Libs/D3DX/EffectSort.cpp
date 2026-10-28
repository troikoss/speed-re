#include "EffectSort.h"

// Original 0x65b64b..0x65b67f. Compare pointer-array entries by original bank
// priority, then the unsigned register-start words. Equal entries remain equal.
extern "C" int __cdecl D3DXEffectCompareConstants(const void *a,const void *b) {
    const unsigned short *left=*reinterpret_cast<const unsigned short *const *>(a),*right=*reinterpret_cast<const unsigned short *const *>(b);
    int result=static_cast<int>(ug2FxConstantOrder7B5048[left[2]]-ug2FxConstantOrder7B5048[right[2]]);
    if (!result) result=static_cast<int>(left[3])-static_cast<int>(right[3]);
    return result;
}
// The original byte swaps are inlined in both CRT bodies. Retain their forward
// byte order, without adding a recovered helper call or a host library sort.
#define FX_SORT_SWAP(a,b) do { \
    unsigned char *left=(a),*right=(b); \
    if (left!=right && width) for (unsigned int byte=0;byte<width;++byte) { \
        unsigned char value=left[byte];left[byte]=right[byte];right[byte]=value; \
    } \
} while (0)
// Original 0x75cc00..0x75cc6c, private CRT shortsort. The original EAX high
// argument is an ordinary explicit source argument here; no registers are forced.
extern "C" void __cdecl D3DXEffectShortSort(unsigned char *low,unsigned char *high,unsigned int width,D3DXEffectCompare compare) {
    while (low<high) {
        unsigned char *maximum=low;
        for (unsigned char *at=low+width;at<=high;at+=width) if (compare(at,maximum)>0) maximum=at;
        FX_SORT_SWAP(maximum,high);
        high-=width;
    }
}
// Original 0x75cc70..0x75cef2. Median-of-three partitioning, equal-value
// trimming and the original smaller-side iteration / explicit 30-slot stack.
extern "C" void __cdecl D3DXEffectSort(void *base,unsigned int count,unsigned int width,D3DXEffectCompare compare) {
    if (count<2 || !width) return;
    unsigned char *low=static_cast<unsigned char *>(base),*high=low+(count-1)*width;
    unsigned char *lowStack[30],*highStack[30];int top=0;
next_partition:
    while (static_cast<unsigned int>(high-low)/width+1>8) {
        unsigned int size=static_cast<unsigned int>(high-low)/width+1;
        unsigned char *middle=low+(size>>1)*width;
        if (compare(low,middle)>0) FX_SORT_SWAP(low,middle);
        if (compare(low,high)>0) FX_SORT_SWAP(low,high);
        if (compare(middle,high)>0) FX_SORT_SWAP(middle,high);
        unsigned char *first=low,*last=high;
scan_partition:
        if (first<middle) {
            do { first+=width;if (first>=middle) goto scan_after_middle; } while (compare(first,middle)<=0);
            if (first>=middle) goto scan_after_middle;
        } else {
scan_after_middle:
            do { first+=width;if (first>high) break; } while (compare(first,middle)<=0);
        }
        do { last-=width;if (last<=middle) break; } while (compare(last,middle)>0);
        if (first<=last) {
            FX_SORT_SWAP(first,last);
            if (middle==last) middle=first;
            goto scan_partition;
        }
        last+=width;
        if (middle<last) {
            do { last-=width;if (last<=middle) goto trim_before_middle; } while (!compare(last,middle));
            if (last<=middle) goto trim_before_middle;
        } else {
trim_before_middle:
            do { last-=width;if (last<=low) break; } while (!compare(last,middle));
        }
        if (static_cast<int>(last-low)<static_cast<int>(high-first)) {
            if (first<high) { lowStack[top]=first;highStack[top]=high;++top; }
            high=last;
            if (last<=low) goto pop_partition;
            goto next_partition;
        }
        if (low<last) { lowStack[top]=low;highStack[top]=last;++top; }
        low=first;
        if (high<=first) goto pop_partition;
    }
    D3DXEffectShortSort(low,high,width,compare);
pop_partition:
    if (--top<0) return;
    low=lowStack[top];high=highStack[top];goto next_partition;
}
#undef FX_SORT_SWAP
