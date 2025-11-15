//==============================================================================================================================================================================
/// \file
/// \brief     Material interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lExpected.h"
#include "lAsset.h"
#include "lTexture.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Material);
    CLASS_WEAK_PTR_DEF(Material);
    CLASS_PTR_DEF(Shader);

    /// Material class
    class Material : public Asset
    {
        CLASS_NO_DEFAULT_CTOR(Material);
        CLASS_NO_COPY_MOVE(Material);
        OBJECT_TYPEINFO;

    public:
        /// property value type
        using PropertyValue = std::variant<int, float, Lumen::TexturePtr>;

        /// creates a smart pointer version of the Material
        static Expected<AssetPtr> MakePtr(const std::filesystem::path &path);

        /// save material
        bool Save() const override;

        /// load material
        bool Load() override;

        /// release material
        void Release() override;

        /// set property
        void SetProperty(std::string_view name, const PropertyValue &property);

        /// get property
        [[nodiscard]] Expected<PropertyValue> GetProperty(std::string_view name) const;

        /// get shader
        [[nodiscard]] ShaderPtr GetShader() const;

        /// set shader
        void SetShader(const ShaderPtr &shader);

    private:
        /// constructs a material
        explicit Material(const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
