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
    CLASS_NO_COPY_MOVE(Impl);
    CLASS_PTR_UNIQUEMAKER(Impl);

public:
    /// constructs a behavior
    explicit Impl(Behavior &owner) : mOwner(owner), mEnabled(true) {}

    /// destroys behavior
    ~Impl() = default;

public:
    /// control enabled
    void Enable(bool enable) { mEnabled = enable; }

    /// return enabled
    [[nodiscard]] bool Enabled() { return mEnabled; }

    /// run component
    void Run()
    {
        if (Enabled())
        {
#ifdef EDITOR
            bool doRun = false;
            if (auto entity = mOwner.Entity().lock())
            {
                Application &application = entity->GetApplication();
                doRun = (0.f != entity->GetApplication().DeltaTime()) || (application.GetState() == Application::State::Stopping);
            }
            if (doRun)
            {
                mOwner.Update();
            }
#else
            mOwner.Update();
#endif
        }
    }
private:
    /// owner
    Behavior &mOwner;

    /// behaviour enabled
    bool mEnabled;
};

//==============================================================================================================================================================================

/// constructor
Behavior::Behavior(HashType type, std::string_view name, const EntityWeakPtr &entity) :
    Component(type, name, entity), mImpl(Behavior::Impl::MakeUniquePtr(*this)) {}

/// destructor
Behavior::~Behavior() = default;

/// control enabled
void Behavior::Enable(bool enable) { mImpl->Enable(enable); }

/// return enabled
bool Behavior::Enabled() { return mImpl->Enabled(); }

/// run component
void Behavior::Run() { return mImpl->Run(); }
