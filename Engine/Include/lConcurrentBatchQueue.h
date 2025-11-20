//==============================================================================================================================================================================
/// \file
/// \brief     ConcurrentBatchQueue is a thread safe batch queue
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"

#include <mutex>

/// Lumen namespace
namespace Lumen
{
    /// ConcurrentBatchQueue template class
    template<typename T>
    class ConcurrentBatchQueue
    {
        CLASS_NO_COPY_MOVE(ConcurrentBatchQueue);

    public:
        /// default constructor
        explicit ConcurrentBatchQueue() = default;

        /// push a batch of items
        void PushBatch(std::vector<T> &&batch)
        {
            std::lock_guard<std::mutex> lock(mMtx);
            mBatchQueue.push_back(std::move(batch));
        }

        /// push several batches of items
        void PushBatchQueue(std::list<std::vector<T>> &batchQueue)
        {
            std::lock_guard<std::mutex> lock(mMtx);
            mBatchQueue.splice(mBatchQueue.end(), batchQueue);
        }

        /// pop all batches of items
        void PopBatchQueue(std::list<std::vector<T>> &batchQueue)
        {
            std::lock_guard<std::mutex> lock(mMtx);
            batchQueue.splice(batchQueue.end(), mBatchQueue);
        }

    private:
        /// mutex
        std::mutex mMtx;

        /// batch queue
        std::list<std::vector<T>> mBatchQueue;
    };
}
