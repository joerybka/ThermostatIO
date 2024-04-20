//
// Created by joerr on 20-Apr-24.
//
#include "Arduino.h"

#ifndef THERMOSTATIO_STABLEDEBOUNCER_H
#define THERMOSTATIO_STABLEDEBOUNCER_H

/**
 * A debouncer that will allow for start delay to allow for inconsistent starts
 * to a debounced call (mostly for allowing switches to stabilize), and then a
 * stop/reset delay (also for allowing switches to stabilize).  It will also
 * function as a normal debouncer.
 */
class StableDebouncer {
public:
    static constexpr unsigned long DefaultDebounceMilliseconds = 1000;  // 1 second

    /**
     * Constructor for full default configuration
     */
    StableDebouncer();

    /**
     * Constructor for defining the number of milliseconds to wait before rerunning a debounced function
     * @param bounceMs The number milliseconds to wait between debounced executions
     */
    explicit StableDebouncer(unsigned long bounceMs);

    /**
     * Set the start bounce delay, this will prevent the debounced function from being called for
     * \p startBounceDelayMs with consistent calls to bounce.
     * @param bounceStartDelayMs The number of milliseconds to delay before calling the bounced function
     */
    void SetStartBounceDelay(unsigned long bounceStartDelayMs);

    /**
     * Sets the amount of time in milliseconds before the next call to \a Bounce can initiate
     * the bouncing behavior after \a Reset is called after an execution.
     * @param bounceResetCooldownMs The cooldown for the bouncer in milliseconds
     */
    void SetBounceResetCooldown(unsigned long bounceResetCooldownMs);

    /**
     * Set the bouncer to ensure that the bounced function is executed only once per reset of the
     * debouncer.
     * @param stickyBounce If true, the debouncer will sticky bounce
     */
    void SetStickyBounce(bool stickyBounce);

    /// Pass this method the function that you would like to run, if you need to run a method, wrap it in a lambda
    /// @tparam F The type of the function pointer, be sure this requires no arguments
    /// @param debounceFunction The parameterless function to run if the debouncer is unbounced
    /// @param startBounceDelayMs The number of milliseconds to delay at the start of a bounce before running the bounced function
    template<typename F>
    void Bounce(F debounceFunction) {
        _startBounce();

        if(_shouldExecute()) {
            debounceFunction();
            _setExecuted();
        }
    }

    /**
     * Reset the debouncer.  This will clear any state of bouncing, allowing for sticky debouncers
     * to execute the function from a bounce, and starting any reset cooldowns if applicable.
     */
    void Reset();

private:
    /// State flag to indicate that the debouncer is currently tracking bouncing activity
    bool _isBouncing = false;

    /// State flag to indicate that the debouncer has executed something with the bouncing behavior
    bool _isExecuted = false;

    /// State flag to indicate that the debouncer is in sticky mode, and will only execute the debounced function once per reset
    bool _isStickyBounce = false;

    /// The time at which the last debounce activity started
    unsigned long _bounceStartMs = 0;

    /// The time at which the last execution of the debounced request took place
    unsigned long _lastBounceExecutionMs = 0;

    /// The time at which this debouncer was last reset
    unsigned long _lastResetMs = 0;

    /// The length of time to wait between debounced function invocations
    unsigned long _bounceMs = DefaultDebounceMilliseconds;

    /// The length of time to wait at the start of a debounce cycle to execute the debounced function
    unsigned long _bounceStartDelayMs = 0;

    /// The length of time after the debouncer is reset after having executed something to start any new debouncing activity
    unsigned long _bounceResetCooldownMs = 0;

    /**
     * Run code that will start the object tracking that it is attempting to bounce
     * an action
     */
    void _startBounce() {
        // if we are in cooldown, don't do anything to the state
        if((millis() - _lastResetMs) < _bounceResetCooldownMs) return;

        // if we are not bouncing, start the bounce
        if(!_isBouncing) {
            _isBouncing = true;
            _bounceStartMs = millis();
        }
    }

    /**
     * Check if the object should execute the action defined by the caller.
     * @return True if the function should be called, otherwise false
     */
    bool _shouldExecute() const {
        if(!_isBouncing) return false;  // bouncing is not enabled
        if(_isExecuted && _isStickyBounce) return false;  // we have already executed after the last reset
        if((millis() - _bounceStartMs) < _bounceStartDelayMs) return false;  // we are still in the initial delay
        if((millis() - _lastBounceExecutionMs) < _bounceMs) return false;  // our last execution was within the bounce timeout
        return true;  // otherwise go for it!
    }

    /**
     * Set internal state that something was executed by the debouncer, and the last
     * time at which something was executed.
     */
    void _setExecuted() {
        _isExecuted = true;
        _lastBounceExecutionMs = millis();
    }

    /**
     * Reset all state tracking to the default, save for the last reset tracker, which
     * will be updated with the current millis stamp.
     */
    void _reset() {
        if(_isExecuted) _lastResetMs = millis();

        _isBouncing = false;
        _isExecuted = false;
        _bounceStartMs = 0;
        _lastBounceExecutionMs = 0;
    }
};

#endif //THERMOSTATIO_STABLEDEBOUNCER_H
