//==============================================================================================================================================================================
/// \file
/// \brief     Shader interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lAsset.h"
#include "lEngine.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Shader);

    /// Shader class
    class Shader : public Asset
    {
        CLASS_NO_COPY_MOVE(Shader);
        OBJECT_TYPEINFO;

    public:
        /// creates a smart pointer version of the shader asset
        static Expected<AssetPtr> MakePtr(EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name);

        /// find shader path from name
        static Expected<std::string_view> FindPath(std::string_view name);

        /// destroys shader
        ~Shader() override;

        /// save a shader
        bool Save() const override;

        /// load a shader
        bool Load() override;

        /// release a shader
        void Release() override;

        /// get shader name
        const std::string &Name();

        /// get shader id
        Id::Type GetShaderId();

        /// set shader id
        void SetShaderId(Id::Type shaderId);

    protected:
        /// constructs a shader
        explicit Shader(const EngineWeakPtr &engine, const std::filesystem::path &path, std::string_view name);

    private:
        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
