#pragma once
#include "common.hpp"
inline void* SystemAlloc(size_t num_pages) {
    size_t size = num_pages << PAGE_SHIFT;   // 字节数 = 页数 * 4096
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    return (ptr == MAP_FAILED) ? nullptr : ptr;
}
// ============================================================

// ---------- PageMap1 (单层数组，仅用于小范围，64位不推荐) ----------
template <int BITS>
class TCMalloc_PageMap1 {
private:
    static const int LENGTH = 1 << BITS;
    void** array_;

public:
    typedef uintptr_t Number;

    explicit TCMalloc_PageMap1() {
        size_t size = sizeof(void*) << BITS;
        size_t alignSize = SizeClass::_RoundUp(size, 1 << PAGE_SHIFT);
        array_ = (void**)SystemAlloc(alignSize >> PAGE_SHIFT);
        memset(array_, 0, sizeof(void*) << BITS);
    }

    void* get(Number k) const {
        if ((k >> BITS) > 0) return nullptr;
        return array_[k];
    }

    void set(Number k, void* v) {
        array_[k] = v;
    }
};

// ---------- PageMap2 (二级基数树，适合中等地址空间) ----------
template <int BITS>
class TCMalloc_PageMap2 {
private:
    static const int ROOT_BITS   = 5;
    static const int ROOT_LENGTH = 1 << ROOT_BITS;
    static const int LEAF_BITS   = BITS - ROOT_BITS;
    static const int LEAF_LENGTH = 1 << LEAF_BITS;

    struct Leaf {
        void* values[LEAF_LENGTH];
    };

    Leaf* root_[ROOT_LENGTH];

public:
    typedef uintptr_t Number;

    explicit TCMalloc_PageMap2() {
        memset(root_, 0, sizeof(root_));
        PreallocateMoreMemory();
    }

    void* get(Number k) const {
        const Number i1 = k >> LEAF_BITS;
        const Number i2 = k & (LEAF_LENGTH - 1);
        if ((k >> BITS) > 0 || root_[i1] == nullptr) return nullptr;
        return root_[i1]->values[i2];
    }

    void set(Number k, void* v) {
        const Number i1 = k >> LEAF_BITS;
        const Number i2 = k & (LEAF_LENGTH - 1);
        assert(i1 < ROOT_LENGTH);
        root_[i1]->values[i2] = v;
    }

    bool Ensure(Number start, size_t n) {
        for (Number key = start; key <= start + n - 1;) {
            const Number i1 = key >> LEAF_BITS;
            if (i1 >= ROOT_LENGTH) return false;

            if (root_[i1] == nullptr) {
                size_t size = SizeClass::_RoundUp(sizeof(Leaf), 1 << PAGE_SHIFT);
                void* ptr = SystemAlloc(size >> PAGE_SHIFT);
                if (ptr == nullptr) return false;
                Leaf* leaf = reinterpret_cast<Leaf*>(ptr);
                memset(leaf, 0, sizeof(Leaf));
                root_[i1] = leaf;
            }
            key = ((key >> LEAF_BITS) + 1) << LEAF_BITS;
        }
        return true;
    }

    void PreallocateMoreMemory() {
        Ensure(0, 1 << BITS);
    }
};

// ---------- PageMap3 (三级基数树，支持 64 位大地址空间) ----------
template <int BITS>
class TCMalloc_PageMap3 {
private:
    static const int INTERIOR_BITS = (BITS + 2) / 3;
    static const int INTERIOR_LENGTH = 1 << INTERIOR_BITS;
    static const int LEAF_BITS = BITS - 2 * INTERIOR_BITS;
    static const int LEAF_LENGTH = 1 << LEAF_BITS;

    struct Node {
        void* ptrs[INTERIOR_LENGTH];
    };

    struct Leaf {
        void* values[LEAF_LENGTH];
    };

    Node* root_;

    Node* NewNode() {
        size_t size = SizeClass::_RoundUp(sizeof(Node), 1 << PAGE_SHIFT);
        void* ptr = SystemAlloc(size >> PAGE_SHIFT);
        if (ptr == nullptr) return nullptr;
        Node* node = reinterpret_cast<Node*>(ptr);
        memset(node, 0, sizeof(Node));
        return node;
    }

    Leaf* NewLeaf() {
        size_t size = SizeClass::_RoundUp(sizeof(Leaf), 1 << PAGE_SHIFT);
        void* ptr = SystemAlloc(size >> PAGE_SHIFT);
        if (ptr == nullptr) return nullptr;
        Leaf* leaf = reinterpret_cast<Leaf*>(ptr);
        memset(leaf, 0, sizeof(Leaf));
        return leaf;
    }

public:
    typedef uintptr_t Number;

    explicit TCMalloc_PageMap3() {
        root_ = NewNode();
    }

    void* get(Number k) const {
        const Number i1 = k >> (LEAF_BITS + INTERIOR_BITS);
        const Number i2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1);
        const Number i3 = k & (LEAF_LENGTH - 1);

        if ((k >> BITS) > 0) return nullptr;
        Node* node1 = root_;
        if (node1 == nullptr || node1->ptrs[i1] == nullptr) return nullptr;
        Node* node2 = reinterpret_cast<Node*>(node1->ptrs[i1]);
        if (node2->ptrs[i2] == nullptr) return nullptr;
        Leaf* leaf = reinterpret_cast<Leaf*>(node2->ptrs[i2]);
        return leaf->values[i3];
    }

    void set(Number k, void* v) {
        assert(k >> BITS == 0);
        const Number i1 = k >> (LEAF_BITS + INTERIOR_BITS);
        const Number i2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1);
        const Number i3 = k & (LEAF_LENGTH - 1);
        Node* node1 = root_;
        Node* node2 = reinterpret_cast<Node*>(node1->ptrs[i1]);
        Leaf* leaf = reinterpret_cast<Leaf*>(node2->ptrs[i2]);
        leaf->values[i3] = v;
    }

    bool Ensure(Number start, size_t n) {
        for (Number key = start; key <= start + n - 1;) {
            const Number i1 = key >> (LEAF_BITS + INTERIOR_BITS);
            const Number i2 = (key >> LEAF_BITS) & (INTERIOR_LENGTH - 1);

            if (i1 >= INTERIOR_LENGTH || i2 >= INTERIOR_LENGTH) return false;

            if (root_->ptrs[i1] == nullptr) {
                Node* node = NewNode();
                if (node == nullptr) return false;
                root_->ptrs[i1] = node;
            }

            Node* level2 = reinterpret_cast<Node*>(root_->ptrs[i1]);
            if (level2->ptrs[i2] == nullptr) {
                Leaf* leaf = NewLeaf();
                if (leaf == nullptr) return false;
                level2->ptrs[i2] = leaf;
            }

            key = ((key >> LEAF_BITS) + 1) << LEAF_BITS;
        }
        return true;
    }

    void PreallocateMoreMemory() {}
};