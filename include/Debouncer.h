#include "Arduino.h"

#ifndef DEBOUNCER_H
#define DEBOUNCER_H

/// @brief Class that will only execute the function that is passed to it on the frequncy set by the bounce time.
class Debouncer {
  private:
    /// @brief The last bounce time in milliseconds, populated with the millis() function
    unsigned long _lastBounce = 0;

    /// @brief True if this bouncer has invoked any functions and not been reset
    bool _permaBounced = false;

    /// @brief The configured bounce time in this instance
    unsigned long _bounceTimeMs;

    /// @brief Set the last bounce timer to the current millis()
    void _setLastBounce() {
      _lastBounce = millis();
      _permaBounced = true;
    }

  public:
    /// @brief The default value for the bounce time if no configuration is passed
    const unsigned long DefaultBounceTimeMs = 1000;  // 1 second

    /// @brief Instantiate the debouncer with the default bounce time of 1000 milliseconds
    Debouncer();

    /// @brief Instantiate the debouncer with a custom bounce timer
    /// @param bounceTimeMs The time in milliseconds to wait between function invocations
    explicit Debouncer(unsigned long bounceTimeMs);

    /// @brief Pass this method the function that you would like to run, if you need to run a method, wrap it in a lambda
    /// @tparam F The type of the function pointer, be sure this requires no arguments
    /// @param debounceFunction The parameterless function to run if the debouncer is unbounced
    template<typename F>
    void Bounce(F debounceFunction) { 
      if(!IsBounced()) {
        debounceFunction();
        _setLastBounce();
      }
    }

    /// @brief Execute the running of any functions with this debouncer until it is manually reset
    /// @tparam F The type of the function pointer, be sure this requires no arguments
    /// @param debounceFunction The parameterless function to run if the debouncer has been reset or is in its initial state
    template<typename F>
    void PermaBounce(F debounceFunction) {
      if(!IsPermaBounced()) {
        debounceFunction();
        _setLastBounce();
      }
    }

    /// @brief Accessor to check if this instance will execute functions
    /// @return False if this debouncer would execute the function passed to Execute, otherwise true
    bool IsBounced();

    /// @brief Accessor to check if this instance has executed any functions and not been reset
    /// @return False if no function has been run since instantiation or the last reset, otherwise true
    bool IsPermaBounced();
    
    /// @brief Reset the last bounce and perma-bounce to their initial values
    void Reset();
};

#endif