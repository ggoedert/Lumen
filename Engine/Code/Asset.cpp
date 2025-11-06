//==============================================================================================================================================================================
/// \file
/// \brief     Asset
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lAsset.h"

using namespace Lumen;

/// Asset::Impl class
class Asset::Impl
{
    CLASS_NO_DEFAULT_CTOR(Impl);
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs a asset
    explicit Impl(std::string_view name, const std::filesystem::path &path) : mName(name), mPath(path) {}

    /// destroys behavior
    ~Impl() = default;

    /// get name
    const std::string &Name() { return mName; }

    /// get path
    const std::filesystem::path &Path() { return mPath; }

private:
    /// name
    const std::string mName;

    /// path
    const std::filesystem::path mPath;
};

//==============================================================================================================================================================================

/// constructor a asset
Asset::Asset(HashType type, std::string_view name, const std::filesystem::path &path) : Object(type), mImpl(Asset::Impl::MakeUniquePtr(name, path)) {}

/// destructor asset
Asset::~Asset() = default;

/// get name
const std::string &Asset::Name()
{
    return mImpl->Name();
}

/// get path
const std::filesystem::path &Asset::Path()
{
    return mImpl->Path();
}
