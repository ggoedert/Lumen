//==============================================================================================================================================================================
/// \file
/// \brief     FileSystemResources interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lAssetManager.h"

/// Lumen namespace
namespace Lumen
{
    /// FileSystemResources class
    class FileSystemResources : public AssetFactory
    {
        CLASS_NO_DEFAULT_CTOR(FileSystemResources);
        CLASS_NO_COPY_MOVE(FileSystemResources);

    public:
        /// creates a smart pointer version of the folder resources
        static AssetFactoryPtr MakePtr(float priority);

        /// check if asset exists
        [[nodiscard]] bool Exists(const std::filesystem::path &path) const override;

        /// import asset
        [[nodiscard]] Expected<AssetPtr> Import(EngineWeakPtr &engine, HashType type, const std::filesystem::path &path) const override;

    private:
        /// constructor
        explicit FileSystemResources(float priority);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
