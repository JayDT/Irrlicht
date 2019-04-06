#include "NsApp/DispatcherTimer.h"
#include "CIrrDeviceStub.h"

using namespace NoesisApp;

uint32_t DispatcherTimer::m_nextID = 0;

DispatcherTimer::~DispatcherTimer()
{
	Stop();
}

DispatcherTimer::DispatcherTimer(irr::IrrlichtDevice* device)
    : m_ownerDevice(device)
	, m_timer(++m_nextID)
{
}

DispatcherTimer::DispatcherTimer(irr::IrrlichtDevice* device, uint32_t interval, std::function<void()> const& callback)
    : m_ownerDevice(device)
	, m_timer(++m_nextID)
{
    SetInterval(interval);
    Tick += [callback](void*, System::Events::EventArg& e)
    {
        callback();
    };
}

void DispatcherTimer::Finalize()
{
    if (m_timer)
    {
        Stop();
    }
}

void DispatcherTimer::SetInterval(const uint32_t value)
{
    bool enabled = GetIsEnabled();
    Stop();
    m_interval = value;
    SetIsEnabled(enabled);
}

void DispatcherTimer::SetIsEnabled(const bool value)
{
    if (GetIsEnabled() != value)
    {
        if (value)
        {
            Start();
        }
        else
        {
            Stop();
        }
    }
}

void DispatcherTimer::Run(irr::IrrlichtDevice* device, std::function<void()> action, uint32_t interval)
{
    auto timer = std::make_shared<DispatcherTimer>(device, interval, action);

    timer->Start();
}

void DispatcherTimer::RunOnce(irr::IrrlichtDevice* device, std::function<void()> action, uint32_t interval)
{
    auto timer = std::make_shared<DispatcherTimer>(device);

    timer->SetInterval(interval);

    std::weak_ptr<DispatcherTimer> weak_timer = timer;
    timer->Tick += [action, weak_timer](void*, System::Events::EventArg& e)
    {
        action();
        if (!weak_timer.expired())
            weak_timer.lock()->Stop();
    };

    timer->Start();
}

void DispatcherTimer::Start()
{
    if (!GetIsEnabled())
    {
		static_cast<irr::CIrrDeviceStub*>(m_ownerDevice)->setTimer(m_timer, m_interval, [this]()
        {
            if (GetIsEnabled() && Tick)
            {
                System::Events::EventArg e;
                Tick(this, e);
            }
        });

        m_isEnabled = true;
    }
}

void DispatcherTimer::Stop()
{
    if (GetIsEnabled())
    {
		static_cast<irr::CIrrDeviceStub*>(m_ownerDevice)->unsetTimer(m_timer);
        m_isEnabled = false;
    }
}
