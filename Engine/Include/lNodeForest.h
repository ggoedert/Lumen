//==============================================================================================================================================================================
/// \file
/// \brief     NodeForest is an stl style container template that stores data hierarchy in a tree structure, with multiple root nodes allowed (a forest)
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

/// Lumen namespace
namespace Lumen
{
    /// NodeForest template class
    template<typename T>
    class NodeForest
    {
        CLASS_NO_COPY_MOVE(NodeForest);

    public:
        /// key type
        using KeyType = size_t;

        /// node struct
        struct Node
        {
            /// parent node key
            KeyType mParentKey;

            /// child nodes keys
            std::vector<KeyType> mChildKeys;

            /// node data
            T mData;
        };

        /// iterator
        using iterator = typename std::unordered_map<KeyType, Node>::iterator;

        /// const iterator
        using const_iterator = typename std::unordered_map<KeyType, Node>::const_iterator;

        /// no parent key
        static constexpr KeyType NoKey = static_cast<KeyType>(SIZE_MAX);

        /// default constructor
        explicit NodeForest() = default;

        /// begin iterator
        [[nodiscard]] iterator begin() { return mNodes.begin(); }

        /// const begin iterator
        [[nodiscard]] const_iterator begin() const { return mNodes.begin(); }

        /// cbegin iterator
        [[nodiscard]] const_iterator cbegin() const { return mNodes.cbegin(); }

        /// end iterator
        [[nodiscard]] iterator end() { return mNodes.end(); }

        /// const end iterator
        [[nodiscard]] const_iterator end() const { return mNodes.end(); }

        /// cend iterator
        [[nodiscard]] const_iterator cend() const { return mNodes.cend(); }

        /// stores data and returns iterator to the inserted node
        iterator insert(const KeyType parentKey, std::convertible_to<T> auto &&data)
        {
            mLastKey = (mLastKey != NoKey) ? mLastKey + 1 : 0;

            // insert the new node
            auto nodeIt = mNodes.emplace(
                mLastKey,
                Node { parentKey, {}, std::forward<decltype(data)>(data) }
            ).first;

            if (parentKey != NoKey)
            {
                auto parentIt = mNodes.find(parentKey);
                L_ASSERT(parentIt != mNodes.end());
                parentIt->second.mChildKeys.push_back(mLastKey);
            }
            else
            {
                mRoots.push_back(mLastKey);
            }

            return nodeIt;
        }

        /// erase a node and all its descendants, returns number of nodes removed
        size_t erase(const KeyType key)
        {
            auto it = mNodes.find(key);
            if (it == mNodes.end())
                return 0;

            // remove this key from its parent's child list
            const KeyType parentKey = it->second.mParentKey;
            if (parentKey != NoKey)
            {
                auto parentIt = mNodes.find(parentKey);
                if (parentIt != mNodes.end())
                {
                    std::erase(parentIt->second.mChildKeys, key);
                }
            }
            else
            {
                std::erase(mRoots, key);
            }

            // collect subtree keys and erase from mNodes
            size_t removedCount = 0;
            std::vector<KeyType> stack = { key };
            do
            {
                KeyType cur = stack.back();
                stack.pop_back();

                auto curIt = mNodes.find(cur);
                if (curIt != mNodes.end())
                {
                    // add children to stack to delete them next
                    for (const auto &childKey : curIt->second.mChildKeys)
                        stack.push_back(childKey);

                    mNodes.erase(curIt);
                    removedCount++;
                }
            }
            while (!stack.empty());

            return removedCount;
        }

        /// find a node by its key
        [[nodiscard]] iterator find(const KeyType key) { return mNodes.find(key); }

        /// const find a node by its key
        [[nodiscard]] const_iterator find(const KeyType key) const { return mNodes.find(key); }

        /// find a node by a predicate
        template<class Predicate>
        [[nodiscard]] iterator find_if(Predicate p)
        {
            auto it = mNodes.begin();
            for (; it != mNodes.end(); ++it)
                if (p(*it))
                    return it;
            return it;
        }

        /// const find a node by a predicate
        template<class Predicate>
        [[nodiscard]] const_iterator find_if(Predicate p) const
        {
            auto it = mNodes.cbegin();
            for (; it != mNodes.cend(); ++it)
                if (p(*it))
                    return it;
            return it;
        }

        /// returns the list of root node keys
        [[nodiscard]] const std::vector<KeyType> &roots() const noexcept { return mRoots; }

    private:
        /// nodes
        std::unordered_map<KeyType, Node> mNodes;

        /// last key used
        KeyType mLastKey = NoKey;

        /// root nodes keys
        std::vector<KeyType> mRoots;
    };
}
