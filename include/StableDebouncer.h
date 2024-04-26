//
// Created by joerr on 20-Apr-24.
//
#include "Arduino.h"

#ifndef THERMOSTATIO_STABLEDEBOUNCER_H
#define THERMOSTATIO_STABLEDEBOUNCER_H

enum StableDebouncerState {
    Idle = 0,  // the debouncer has not been used, or does not have a reset cooldown
    StartDelay = 1,  // the debouncer is waiting for calls to execute to stabilize before allowing execution
    Executing = 2,  // allowing execution at most once every X milliseconds
    Executed = 5,  // the debouncer is sticky and has blown its wad
    StopDelay = 3,  // the debouncer is waiting for calls to reset to stabilize before allowing rull reset
    ResetCooldown = 4,  // The debouncer is in reset cooldown, it will not allow initiating a debounce flow for X milliseconds
};

enum StableDebounceProgressType {
    TryExecute = 0,  // Execute was called, try to progress state with execution rules
    TryReset = 1,  // Reset was called, try to progress state with reset rules
};

/**
 * A debouncer that will allow for start delay to allow for inconsistent starts
 * to a debounced call (mostly for allowing switches to stabilize), and then a
 * stop/reset delay (also for allowing switches to stabilize).  It will also
 * function as a normal debouncer.
 */
class StableDebouncer {
public:
    static constexpr unsigned long DefaultFrequencyMilliseconds = 1000;  // 1 second

    /**
     * Constructor for full default configuration
     */
    StableDebouncer();

    /**
     * Constructor for defining the number of milliseconds to wait before rerunning a debounced function
     * @param frequencyMs The number milliseconds to wait between debounced executions
     */
    explicit StableDebouncer(unsigned long frequencyMs);

    /**
     * Set the start debounce delay, this will prevent the debounced function from being called for
     * \p startBounceDelayMs with consistent calls to \a Execute.
     * @param startDelayMs The number of milliseconds to delay before allowing \a Execute to run the function
     */
    void SetStartDelay(unsigned long startDelayMs);

    /**
     * Set the amount of time the debouncer will wait for consistent calls to reset before fully
     * resetting itself and allowing another initiation flow or starting a reset cooldown
     * @param stopDelayMs The number of milliseconds to delay full reset
     */
    void SetStopDelay(unsigned long stopDelayMs);

    /**
     * Sets the amount of time in milliseconds before the next call to \a Execute can initiate
     * the bouncing behavior after \a Reset is called after an execution.
     * @param bounceResetCooldownMs The cooldown for the bouncer in milliseconds
     */
    void SetResetCooldown(unsigned long bounceResetCooldownMs);

    /**
     * Set the debouncer to ensure that the executed function is executed only once per reset of the
     * debouncer.
     * @param stickyBounce If true, the debouncer will sticky bounce
     */
    void SetStickyBounce(bool stickyBounce);

    /// Pass this method the function that you would like to run, if you need to run a method, wrap it in a lambda
    /// @tparam F The type of the function pointer, be sure this requires no arguments
    /// @param debounceFunction The parameterless function to run if the debouncer is unbounced
    /// @param startBounceDelayMs The number of milliseconds to delay at the start of a bounce before running the bounced function
    template<typename F>
    void Execute(F debounceFunction) {
        _advanceExecute();

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
    StableDebouncerState _state = Idle;

    /// State flag to indicate that the debouncer is in sticky mode, and will only execute the debounced function once per reset
    bool _isStickyBounce = false;

    /// The time at which the last debounce activity started
    unsigned long _debounceStartExecuteRequestMs = 0;

    /// The time at which the last execution of the debounced request took place
    unsigned long _lastExecutionMs = 0;

    /// The time at which the last "uninterrupted" request to reset came in
    unsigned long _debounceStopExecuteRequestMs = 0;

    /// The time at which this debouncer was last reset
    unsigned long _lastResetMs = 0;

    /// The length of time to wait before repeating function invocations
    unsigned long _frequencyMs = DefaultFrequencyMilliseconds;

    /// The length of time to wait at the start of a debounce cycle to execute the debounced function
    unsigned long _debounceStartExecuteDelayMs = 0;

    /// The length of time to wait for consistent calls to reset to fully reset and start cooldown or switch to Idle
    unsigned long _debounceStopExecuteDelayMs = 0;

    /// The length of time after the debouncer is reset after having executed something to start any new debouncing activity
    unsigned long _bounceResetCooldownMs = 0;

    /// is the amount of time that has passed since the start of the current debounce request greater than the delay?
    bool _isPastStartDelay() { return (millis() - _debounceStartExecuteRequestMs) >= _debounceStartExecuteDelayMs; }

    /// is the amount of time that has passed since the last consistent reset request past the delay
    bool _isPastStopDelay() { return (millis() - _debounceStopExecuteRequestMs) >= _debounceStopExecuteDelayMs; }

    /// is the amount of time since the last successful reset past the cooldown
    bool _isPastResetCooldown() { return (millis() - _lastResetMs) >= _bounceResetCooldownMs; }

    /**
     * Advance the state of the execution status through the flow using the "execute" request
     * parameter
     */
    void _advanceExecute() {
        switch(_state) {
            case Idle:  // if we are currently idle, initiate flow
                _debounceStartExecuteRequestMs = millis();
                _state = _isPastStartDelay() ? Executing : StartDelay;
                break;
            case StartDelay:  // if we are past start delay, we can move to executing
                if(_isPastStartDelay())
                    _state = Executing;
                break;
            case Executing:  // if we are executing, and are sticky, move to executed, otherwise do nothing
                if(_isStickyBounce)
                    _state = Executed;
                break;
            case Executed:  // do nothing in an executed state
                break;
            case StopDelay:  // if we are in stop delay, change back to execute states and reset the stop delay timer
                _state = _isStickyBounce ? Executed : Executing;
                _debounceStopExecuteRequestMs = 0;
            case ResetCooldown:  // if the reset cooldown is over initiate start delay, otherwise do nothing
                if((millis() - _lastResetMs) > _bounceResetCooldownMs) {
                    _state = StartDelay;
                    _debounceStartExecuteRequestMs = millis();
                }
                break;
            default:
                break;
        }
    }

    /**
     * Advance the flow of the debounce using the "reset" request parameter
     */
    void _advanceReset() {
        switch(_state) {
            case Idle:  // we dont do anything to reset an idle state
                break;
            case StartDelay:  // if we are in start delay, switch back to idle and reset timer
                _state = Idle;
                _debounceStartExecuteRequestMs = 0;
                break;
            case Executing:  // if we are executing or have executed, initiate stop delay
            case Executed:
                _state = StopDelay;
                _debounceStopExecuteRequestMs = millis();
                break;
            case StopDelay:  // if we are finished with delay reset
                if(_isPastStopDelay()) {
                    _lastResetMs = millis();
                    _debounceStartExecuteRequestMs = 0;
                    _debounceStopExecuteRequestMs = 0;
                    _state = _isPastResetCooldown() ? Idle : ResetCooldown;
                }
                break;
            case ResetCooldown:
                if(_isPastResetCooldown()) {
                    _lastResetMs = 0;
                    _state = Idle;
                }
                break;
            default:
                break;
        }
    }

    /**
     * Check if the object should execute the action defined by the caller.
     * @return True if the function should be called, otherwise false
     */
    bool _shouldExecute() const {
        if(_state != Executing && _state != StopDelay) return false;  // if we are not in an execution state do not execute
        if((millis() - _lastExecutionMs) < _frequencyMs) return false;  // our last execution was within the bounce timeout
        return true;  // otherwise go for it!
    }

    /**
     * Set internal state that something was executed by the debouncer, and the last
     * time at which something was executed.
     */
    void _setExecuted() {
        _lastExecutionMs = millis();
    }

    /**
     * Set our last reset timestamp
     */
    void _setReset() {
        _lastResetMs = millis();
    }
};

#endif //THERMOSTATIO_STABLEDEBOUNCER_H
