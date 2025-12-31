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
    explicit Impl() : mEnabled(true) {}

    /// destroys behavior
    ~Impl() = default;

public:
    /// set owner
    void SetOwner(BehaviorWeakPtr owner) { mOwner = owner; }

    /// control enabled
    void Enable(bool enable) { mEnabled = enable; }

    /// return enabled
    [[nodiscard]] bool Enabled() { return mEnabled; }

    /// run component
    void Run()
    {
        if (Enabled())
        {
            if (auto owner = mOwner.lock())
            {
                bool doRun = false;
                if (auto entity = owner->Entity().lock())
                {
                    Application &application = entity->GetApplication();
                    doRun = (0.f != entity->GetApplication().DeltaTime()) || (application.GetState() == Application::State::Stopping);
                }
                if (doRun)
                {
                    owner->Update();
                }
            }
        }
    }
private:
    /// owner
    BehaviorWeakPtr mOwner;

    /// behaviour enabled
    bool mEnabled;
};

//==============================================================================================================================================================================

/// constructor
Behavior::Behavior(HashType type, std::string_view name, const EntityWeakPtr &entity) :
    Component(type, name, entity), mImpl(Behavior::Impl::MakeUniquePtr()) {}

/// destructor
Behavior::~Behavior() = default;

/// initialize behavior
void Behavior::Initialize() { mImpl->SetOwner(weak_from_this()); }

/// control enabled
void Behavior::Enable(bool enable) { mImpl->Enable(enable); }

/// return enabled
bool Behavior::Enabled() { return mImpl->Enabled(); }

/// run component
void Behavior::Run() { return mImpl->Run(); }
