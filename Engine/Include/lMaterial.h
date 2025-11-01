//==============================================================================================================================================================================
/// \file
/// \brief     Material interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lExpected.h"
#include "lAsset.h"
#include "lFileSystem.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Material);
    CLASS_PTR_DEF(Shader);

    /// Material class
    class Material : public Asset
    {
        CLASS_NO_DEFAULT_CTOR(Material);
        CLASS_NO_COPY_MOVE(Material);
        OBJECT_TYPEINFO;

    public:
        /// creates a smart pointer version of the Material
        static Expected<AssetPtr> MakePtr(const std::filesystem::path &path, std::string_view name);

        /// get shader
        [[nodiscard]] ShaderPtr GetShader() const;

        /// set shader
        void SetShader(const ShaderPtr &shader);

    private:
        /// constructs a material with an shader
        explicit Material(ShaderPtr shader, const std::filesystem::path &path, std::string_view name);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
