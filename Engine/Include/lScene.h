//==============================================================================================================================================================================
/// \file
/// \brief     scene interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lExpected.h"
#include "lAsset.h"

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Scene);
    CLASS_WEAK_PTR_DEF(Scene);
    CLASS_PTR_DEF(Application);

    /// Scene class
    class Scene : public Asset
    {
        CLASS_NO_DEFAULT_CTOR(Scene);
        CLASS_NO_COPY_MOVE(Scene);
        OBJECT_TYPEINFO;

    public:
        /// destructor
        ~Scene() override;

        /// creates a smart pointer version of the scene
        static ScenePtr MakePtr(Application &application, const std::filesystem::path &path);

        /// register scene name / path
        static void Register(std::string_view name, std::string_view path);

        /// find scene path from name
        static Expected<std::string_view> Find(std::string_view name);

        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed);

        /// save scene
        bool Save() const override;

        /// load scene
        bool Load() override;

        /// release scene
        void Release() override;

    private:
        /// constructor
        explicit Scene(Lumen::Application &application, const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
