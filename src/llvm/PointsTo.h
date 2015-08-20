#ifndef _LLVM_POINTS_TO_ANALYSIS_H_
#define _LLVM_POINTS_TO_ANALYSIS_H_

#include "analysis/DataFlowAnalysis.h"

namespace dg {

class LLVMDependenceGraph;
class LLVMNode;

namespace analysis {

#define UNKNOWN_OFFSET ~((uint64_t) 0)

// just a wrapper around uint64_t to
// handle UNKNOWN_OFFSET somehow easily
// maybe later we'll make it a range
struct Offset
{
    Offset(uint64_t o = UNKNOWN_OFFSET) : offset(o) {}
    Offset& operator+=(const Offset& o)
    {
        if (offset == UNKNOWN_OFFSET)
            return *this;

        if (o.offset == UNKNOWN_OFFSET)
            offset = UNKNOWN_OFFSET;
        else
            offset += o.offset;

        return *this;
    }

    Offset operator+(const Offset& o)
    {
        if (offset == UNKNOWN_OFFSET || o.offset == UNKNOWN_OFFSET)
            return UNKNOWN_OFFSET;

        return Offset(offset + o.offset);
    }

    bool operator<(const Offset& o) const
    {
        return offset < o.offset;
    }

    bool isUnknown() const { return offset == UNKNOWN_OFFSET; }

    uint64_t operator*() const { return offset; }
    const uint64_t *operator->() const { return &offset; }

    uint64_t offset;
};

struct MemoryObj;
struct Pointer
{
    Pointer(MemoryObj *m, Offset off = 0) : obj(m), offset(off) {}

    MemoryObj *obj;
    Offset offset;

    bool operator<(const Pointer& oth) const
    {
        return obj == oth.obj ? offset < oth.offset : obj < oth.obj;
    }
};

typedef std::set<Pointer> PointsToSetT;
typedef std::set<LLVMNode *> ValuesSetT;
typedef std::map<Offset, PointsToSetT> PointsToMapT;
typedef std::map<Offset, ValuesSetT> ValuesMapT;

struct MemoryObj
{
    MemoryObj(LLVMNode *n) : node(n) {}
    LLVMNode *node;

    PointsToMapT pointsTo;

    bool addPointsTo(const Offset& off, const Pointer& ptr)
    {
        if (isUnknown())
            return false;

        return pointsTo[off].insert(ptr).second;
    }

    bool isUnknown() const { return node == nullptr; }
    bool setUnknown()
    {
        if (isUnknown())
            return false;

        pointsTo.clear();
        node = nullptr;
        return true;
    }
#if 0
    ValuesMapT values;
    // actually we should take offsets into
    // account, since we can memset only part of the memory
    ValuesSetT memsetTo;
#endif
};

class LLVMPointsToAnalysis : public DataFlowAnalysis<LLVMNode>
{
public:
    LLVMPointsToAnalysis(LLVMDependenceGraph *dg);

    /* virtual */
    bool runOnNode(LLVMNode *node);
};

} // namespace analysis
} // namespace dg

#endif //  _LLVM_POINTS_TO_ANALYSIS_H_