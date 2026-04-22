//==============================================================================================================================================================================
/// \file
/// \brief     metafile interface
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================
#pragma once

#include "lDefs.h"
#include "lExpected.h"
#include "lSerializedData.h"
#include "lObject.h"

/// \cond
#include <filesystem>
/// \endcond

/// Lumen namespace
namespace Lumen
{
    CLASS_PTR_DEF(Metafile);
    CLASS_WEAK_PTR_DEF(Metafile);

    /// Metafile class
    class Metafile : public Object
    {
        CLASS_NO_DEFAULT_CTOR(Metafile);
        CLASS_NO_COPY_MOVE(Metafile);
        OBJECT_TYPEINFO;

    public:
        /// destructor
        ~Metafile() override;

        /// creates a smart pointer version of the metafile
        static MetafilePtr MakePtr(const std::filesystem::path &path);

        /// register metafile name / path
        static void Register(std::string_view name, std::string_view path);

        /// find metafile path from name
        static Expected<std::string_view> Find(std::string_view name);

        /// serialize
        void Serialize(Serialized::Type &out, bool packed) const;

        /// deserialize
        void Deserialize(const Serialized::Type &in, bool packed);

        /// save metafile
        bool Save() const;

        /// load metafile
        bool Load();

    private:
        /// constructor
        explicit Metafile(const std::filesystem::path &path);

        /// private implementation
        CLASS_PIMPL_DEF(Impl);
    };
}
