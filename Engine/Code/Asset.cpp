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
    explicit Impl(const std::filesystem::path &path) : mPath(path) {}

    /// destructor
    ~Impl() = default;

    /// get path
    const std::filesystem::path &Path() { return mPath; }

private:
    /// path
    const std::filesystem::path mPath;
};

//==============================================================================================================================================================================

/// constructor a asset
Asset::Asset(HashType type, const std::filesystem::path &path) : Object(type), mImpl(Asset::Impl::MakeUniquePtr(path)) {}

/// destructor asset
Asset::~Asset() = default;

/// get path
const std::filesystem::path &Asset::Path()
{
    return mImpl->Path();
}
