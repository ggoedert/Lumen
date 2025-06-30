//==============================================================================================================================================================================
/// \file
/// \brief     Behavior
/// \copyright Copyright (c) Gustavo Goedert. All rights reserved.
//==============================================================================================================================================================================

#include "lBehavior.h"

using namespace Lumen;

/// Behavior::Impl class
class Behavior::Impl
{
    CLASS_NO_COPY_MOVE(Behavior::Impl);
    CLASS_PTR_UNIQUEMAKER(Behavior::Impl);

public:
    /// constructs a behavior
    explicit Impl() : mEnabled(true) {}

    /// destroys behavior
    ~Impl() = default;

public:
    /// control enabled
    void Enable(bool enable) { mEnabled = enable; }

    /// return enabled
    [[nodiscard]] bool Enabled() { return mEnabled; }

private:
    /// behaviour enabled
    bool mEnabled;
};

//==============================================================================================================================================================================

/// constructor
Behavior::Behavior(const HashType type, std::string_view name, const GameObjectWeakPtr &gameObject) :
    Component(type, name, gameObject), mImpl(Behavior::Impl::MakeUniquePtr()) {}

/// destructor
Behavior::~Behavior() noexcept = default;

/// control enabled
void Behavior::Enable(bool enable) { mImpl->Enable(enable); }

/// return enabled
bool Behavior::Enabled() { return mImpl->Enabled(); }
