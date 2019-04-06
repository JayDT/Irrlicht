#ifndef __CF_DISPATCHERTIMER_H__
#define __CF_DISPATCHERTIMER_H__

#include <NsApp/IrrNoesis.h>
#include <standard/events.h>
#include <chrono>

namespace irr
{
	class IrrlichtDevice;
}

namespace NoesisApp
{
    /// <summary>
    /// A timer that uses a <see cref="Dispatcher"/> to fire at a specified interval.
    /// </summary>
    class NS_IRR_NOESIS_API DispatcherTimer
    {
    private:
        static uint32_t m_nextID;
		void* m_tag = nullptr;
		irr::IrrlichtDevice* m_ownerDevice;
        uint32_t m_timer = 0;
        uint32_t m_interval = 0;
        bool m_isEnabled = false;

    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="DispatcherTimer"/> class.
        /// </summary>
		virtual ~DispatcherTimer();

        DispatcherTimer(irr::IrrlichtDevice*);

        /// <summary>
        /// Initializes a new instance of the <see cref="DispatcherTimer"/> class.
        /// </summary>
        /// <param name="interval">The interval at which to tick.</param>
        /// <param name="priority">The priority to use.</param>
        /// <param name="callback">The event to call when the timer ticks.</param>
        DispatcherTimer(irr::IrrlichtDevice*, uint32_t interval, std::function<void()> const& callback);

        /// <summary>
        /// Raised when the timer ticks.
        /// </summary>
        System::Events::EventHandler<> Tick;

		/// <summary>
		/// Gets or sets user-defined data associated with the timer.
		/// </summary>
		void SetTag(void* tag) { m_tag = tag; }
		void* GetTag() const { return m_tag; }

        /// <summary>
        /// Gets or sets the interval at which the timer ticks.
        /// </summary>
        uint32_t GetInterval() const
        {
            return m_interval;
        }

        void SetInterval(const uint32_t value);

        /// <summary>
        /// Gets or sets a value indicating whether the timer is running.
        /// </summary>
        bool GetIsEnabled() const
        {
            return m_isEnabled;
        }

        void SetIsEnabled(const bool value);

        /// <summary>
        /// Starts a new timer.
        /// </summary>
        /// <param name="action">
        /// The method to call on timer tick. If the method returns false, the timer will stop.
        /// </param>
        /// <param name="interval">The interval at which to tick.</param>
        /// <param name="priority">The priority to use.</param>
        static void Run(irr::IrrlichtDevice*, std::function<void()> action, uint32_t interval);

        /// <summary>
        /// Runs a method once, after the specified interval.
        /// </summary>
        /// <param name="action">
        /// The method to call after the interval has elapsed.
        /// </param>
        /// <param name="interval">The interval after which to call the method.</param>
        /// <param name="priority">The priority to use.</param>
        static void RunOnce(irr::IrrlichtDevice*, std::function<void()> action, uint32_t interval);

        /// <summary>
        /// Starts the timer.
        /// </summary>
        void Start();

        /// <summary>
        /// Stops the timer.
        /// </summary>
        void Stop();

    private:
        /// <summary>
        /// Finalizes an instance of the <see cref="DispatcherTimer"/> class.
        /// </summary>
        void Finalize();
    };
}


#endif	//#ifndef __XUI_DISPATCHERTIMER_H__
