// Geometric Tools Library
// https://www.geometrictools.com
// Copyright (c) 2022 David Eberly
// Distributed under the Boost Software License, Version 1.0
// https://www.boost.org/LICENSE_1_0.txt
// File Version: 2022.05.02

#pragma once

// Documentation for this class is
// https://www.geometrictools.com/Documentation/GTLUtility.pdf#MinHeap

#include <cstddef>
#include <limits>
#include <vector>

namespace gtl
{
    // The type T must have a less-than comparison operator<(...) function.
    // The other comparison operators are optional.
    template <typename T>
    class MinHeap
    {
    public:
        static size_t constexpr invalid = std::numeric_limits<size_t>::max();

        // The mNode[].weight values are default constructed. For native T,
        // the weights are uninitialized. For non-native T, the weights are
        // whatever the default constructor creates.
        MinHeap(size_t maxElements = 0)
            :
            mNumElements(0),
            mKeys{},
            mIndices{},
            mNodes{}
        {
            Reset(maxElements);
        }

        // Copy semantics.
        MinHeap(MinHeap const& other) = default;
        MinHeap& operator=(MinHeap const& other) = default;

        // Move semantics.
        MinHeap(MinHeap&& other) = default;
        MinHeap& operator=(MinHeap&& other) = default;

        // Resize the min-heap so that it has the specified maximum number of
        // elements. The previous state of the min-heap is not preserved. The
        // mNode[].weight values are default constructed. For native T, the
        // weights are uninitialized. For non-native T, the weights are
        // whatever the default constructor creates.
        void Reset(size_t maxElements)
        {
            mNumElements = 0;
            if (maxElements > 0)
            {
                mKeys.resize(maxElements);
                mIndices.resize(maxElements);
                mNodes.resize(maxElements);
                for (size_t i = 0; i < maxElements; ++i)
                {
                    mKeys[i] = i;
                    mIndices[i] = i;
                    mNodes[i].handle = invalid;
                    // mNodes[i].weight is unassigned intentionally,
                    // it is invalid.
                }
            }
            else
            {
                mKeys.clear();
                mIndices.clear();
                mNodes.clear();
            }
        }

        // Get the maximum number of elements allowed in the min-heap.
        inline size_t GetMaxElements() const
        {
            return mKeys.size();
        }

        // Get the current number of elements in the min-heap. This number
        // is in the range {0..maxElements}.
        inline size_t GetNumElements() const
        {
            return mNumElements;
        }

        // Get the root of the min-heap. The function reads the root but does
        // not remove the element from the min-heap. The function return is
        // 'key'. If the min-heap is not empty, the 'key' is valid, 'weight'
        // corresponds to the root of the min-heap, and 'handle' is the
        // user-provided identifier for the corresponding application object.
        // If the min-heap is empty, 'key' is invalid and the operation is
        // unsuccessful, in which case both 'handle' and 'weight' are invalid
        // and should not be used.
        size_t GetMinimum(size_t& handle, T& weight) const
        {
            if (mNumElements > 0)
            {
                // Get the minimum value from the root of the min-heap.
                size_t key = mKeys[0];
                auto& node = mNodes[key];
                handle = node.handle;
                weight = node.weight;
                return key;
            }
            else
            {
                handle = invalid;
                // weight is unassigned intentionally, it is invalid.
                return invalid;
            }
        }

        // Insert (handle,weight) into the min-heap. The function return is
        // 'key'. If the min-heap is not full before the insertion, 'key' is
        // valid and (handle,weight) is stored in the appropriate node. If the
        // min-heap is full before the insertion, 'key' is invalid, the
        // operation is unsuccessful, and the min-heap is not modified. When
        // the insertion is successful, the 'key' may be used later in a call
        // to Update.
        size_t Insert(size_t handle, T const& weight)
        {
            if (mNumElements < GetMaxElements())
            {
                // Store (handle, weight) in the last leaf node of the binary
                // tree.
                size_t insertIndex = mNumElements++;
                size_t insertKey = mKeys[insertIndex];
                mIndices[insertKey] = insertIndex;
                auto& node = mNodes[insertKey];
                node.handle = handle;
                node.weight = weight;

                // Propagate the information toward the root of the tree until
                // it reaches its correct position, therefore restoring the
                // tree to a min-heap.
                size_t childIndex = insertIndex;
                while (childIndex > 0)
                {
                    size_t parentIndex = (childIndex - 1) / 2;
                    size_t parentKey = mKeys[parentIndex];
                    if (!(weight < mNodes[parentKey].weight))
                    {
                        // The parent has a weight smaller than or equal to
                        // the child's value. We now have a min-heap.
                        break;
                    }

                    // The parent has a value larger than the child's value.
                    // Swap the parent and child.
                    mKeys[parentIndex] = insertKey;
                    mKeys[childIndex] = parentKey;
                    mIndices[parentKey] = childIndex;
                    mIndices[insertKey] = parentIndex;

                    // Propagate upward.
                    childIndex = parentIndex;
                }

                return insertKey;
            }
            else
            {
                return invalid;
            }
        }

        // Remove the root of the min-heap, which contains the minimum weight.
        // The function return is 'key'. If the min-heap is not empty before
        // the removal, 'key' is valid and corresponds to the node storing
        // (handle, weight). If the min-heap is empty before the removal,
        //  'key' is invalid, the operation is unsuccessful, and the min-heap
        // is not modified. On return, 'key' is valid until another insert,
        // remove or update call is made. The intent is for the caller to use
        // 'key' and, if necessary, clean up any resources that are associated
        // with 'key' before any additional heap-modifying calls.
        size_t Remove(size_t& handle, T& weight)
        {
            if (mNumElements > 0)
            {
                // Get the minimum value from the root of the min-heap.
                size_t removeKey = mKeys[0];
                Node const& node = mNodes[removeKey];
                handle = node.handle;
                weight = node.weight;

                // Get the index of the last occupied leaf node of the
                // min-heap.
                size_t lastIndex = --mNumElements;
                if (lastIndex == 0)
                {
                    // The min-heap has only a single node. No additional work
                    // is required to remove it. The min-heap is now empty.
                    // Restore the keys and indices to their defaults in order
                    // to guarantee deterministic results if the same sequence
                    // of insertions, removals and updates are applied twice.
                    if (mNumElements == 0)
                    {
                        size_t const maxElements = GetMaxElements();
                        for (size_t i = 0; i < maxElements; ++i)
                        {
                            mKeys[i] = i;
                            mIndices[i] = i;
                            mNodes[i].handle = invalid;
                        }
                    }

                    return removeKey;
                }

                // Restore the tree to a min-heap. The last occupied leaf node
                // is moved to the root of the tree in order to preserve the
                // invariant that the unoccupied leaf nodes are always at the
                // end of the array of leaves. After the move, the tree needs
                // to be restored to a min-heap. The root-node value is
                // replaced by the minimum weight of the child nodes. The
                // process is repeated with the child node of minimum weight;
                // its weight is replaced by the minimum value of its child
                // nodes. The traversal continues until the binary tree is
                // restored to a min-heap.
                size_t lastKey = mKeys[lastIndex];
                T const& lastWeight = mNodes[lastKey].weight;

                // Swap the root node and the last leaf node. The lastIndex is
                // decremented so that the old root value is now in the first
                // unoccupied leaf node of the new tree.
                mKeys[0] = lastKey;
                mKeys[lastIndex] = removeKey;
                mIndices[removeKey] = lastIndex;
                mIndices[lastKey] = 0;
                --lastIndex;

                // Propagate the new root value downward until the tree is
                // restored to a min-heap.
                size_t parentIndex = 0;
                size_t childIndex = 1;
                while (childIndex <= lastIndex)
                {
                    size_t childKey = mKeys[childIndex];
                    if (childIndex < lastIndex)
                    {
                        // Select the child with smallest value.
                        size_t otherChildIndex = childIndex + 1;
                        size_t otherChildKey = mKeys[otherChildIndex];
                        if (mNodes[otherChildKey].weight < mNodes[childKey].weight)
                        {
                            childIndex = otherChildIndex;
                            childKey = otherChildKey;
                        }
                    }

                    if (!(mNodes[childKey].weight < lastWeight))
                    {
                        // The tree is now a min-heap.
                        break;
                    }

                    // Move the child into the parent's slot.
                    mKeys[parentIndex] = childKey;
                    mKeys[childIndex] = lastKey;
                    mIndices[lastKey] = childIndex;
                    mIndices[childKey] = parentIndex;

                    // Propagate downward.
                    parentIndex = childIndex;
                    childIndex = 2 * childIndex + 1;
                }

                // The minimum value is now stored in the last occupied
                // leaf node of the old tree, but now the first unoccupied
                // leaf node of the new tree.
                return removeKey;
            }
            else
            {
                handle = invalid;
                // weight is unassigned intentionally, it is invalid.
                return invalid;
            }
        }

        // The value of a min-heap node must be modified via this function
        // call. The side effect is that the binary tree is restored to a
        // min-heap. The input 'updateKey' should be a key returned by some
        // call to "key = Insert(handle, oldWeight)". The input 'updateWeight'
        // is the new value to be associated with that key (and handle). The
        // function return is 'updateKey' when it is valid; that is, it is
        // required that 0 <= updateKey < GetMaxElements(), and internally
        // 0 <= updateIndex < GetNumElements(). The function returns 'true'
        // if and only if the update key is in the required range, in which
        // case the min-heap was not modified.
        bool Update(size_t updateKey, T const& updateWeight)
        {
            size_t updateIndex = mIndices[updateKey];
            if (updateKey < GetMaxElements() && updateIndex < GetNumElements())
            {
                T oldWeight = mNodes[updateKey].weight;
                if (oldWeight < updateWeight)
                {
                    mNodes[updateKey].weight = updateWeight;

                    // The new value is larger than the old value. Propagate
                    // it toward the leaves.
                    size_t parentIndex = mIndices[updateKey];
                    size_t childIndex = 2 * parentIndex + 1;
                    while (childIndex < mNumElements)
                    {
                        // Select the child with smallest value.
                        size_t childKey = mKeys[childIndex];
                        size_t otherChildIndex = childIndex + 1;
                        if (otherChildIndex < mNumElements)
                        {
                            size_t otherChildKey = mKeys[otherChildIndex];
                            if (mNodes[otherChildKey].weight < mNodes[childKey].weight)
                            {
                                childIndex = otherChildIndex;
                                childKey = otherChildKey;
                            }
                        }

                        if (!(mNodes[childKey].weight < updateWeight))
                        {
                            // The new value is in the correct place to
                            // restore the tree to a min-heap. TODO: Is it
                            // possible to reach this block of code?
                            break;
                        }

                        // The child has a value larger than the parent's
                        // value. Swap the parent and child:
                        mKeys[parentIndex] = childKey;
                        mKeys[childIndex] = updateKey;
                        mIndices[updateKey] = childIndex;
                        mIndices[childKey] = parentIndex;

                        // Propagate downward.
                        parentIndex = childIndex;
                        childIndex = 2 * childIndex + 1;
                    }
                }
                else if (updateWeight < oldWeight)
                {
                    mNodes[updateKey].weight = updateWeight;

                    // The new value is smaller than the old value. Propagate
                    // it toward the root.
                    size_t childIndex = updateIndex;
                    while (childIndex > 0)
                    {
                        size_t parentIndex = (childIndex - 1) / 2;
                        size_t parentKey = mKeys[parentIndex];
                        if (!(updateWeight < mNodes[parentKey].weight))
                        {
                            // The new value is in the correct place to
                            // restore the tree to a min-heap heap.
                            break;
                        }

                        // The parent has a value smaller than the child's
                        // value. Swap the child and parent.
                        mKeys[parentIndex] = updateKey;
                        mKeys[childIndex] = parentKey;
                        mIndices[parentKey] = childIndex;
                        mIndices[updateKey] = parentIndex;

                        // Propagate upward.
                        childIndex = parentIndex;
                    }
                }

                return true;
            }
            else
            {
                return false;
            }
        }

        // Support for debugging. The functions test whether the data
        // structure is a valid min-heap.
        bool IsValid() const
        {
            for (size_t childIndex = 1; childIndex < mNumElements; ++childIndex)
            {
                size_t childKey = mKeys[childIndex];
                size_t parentIndex = (childIndex - 1) / 2;
                size_t parentKey = mKeys[parentIndex];
                if (mNodes[childKey].weight < mNodes[parentKey].weight)
                {
                    return false;
                }

                if (mIndices[parentKey] != parentIndex)
                {
                    return false;
                }
            }

            return true;
        }

        // The user-specified information stored in the binary tree nodes.
        // The first of the pair is a handle for the application object
        // involved in the min-heap operation. The second of the pair is
        // the weight associated with that object.
        struct Node
        {
            Node()
                :
                handle(),
                weight{}
            {
            }

            Node(size_t inHandle, T const& inWeight)
                :
                handle(inHandle),
                weight(inWeight)
            {
            }

            size_t handle;
            T weight;
        };

        inline std::vector<Node> const& GetNodes() const
        {
            return mNodes;
        }

        inline Node const& GetNode(size_t key) const
        {
            return mNodes[key];
        }

        inline size_t GetHandle(size_t key) const
        {
            return mNodes[key].handle;
        }

        inline T const& GetWeight(size_t key) const
        {
            return mNodes[key].weight;
        }

    private:
        // Support for the binary tree topology and sorting.
        size_t mNumElements;
        std::vector<size_t> mKeys, mIndices;

        // User-specified information at the nodes of the binary tree.
        std::vector<Node> mNodes;

        friend class UnitTestMinHeap;
    };
}
