//
// Created by joerr on 20-Apr-24.
//

#include "StableDebouncer.h"

StableDebouncer::StableDebouncer() : StableDebouncer(DefaultFrequencyMilliseconds) { }

StableDebouncer::StableDebouncer(unsigned long frequencyMs) {
    _frequencyMs = frequencyMs;
}

void StableDebouncer::SetStartDelay(unsigned long startDelayMs) {
    _debounceStartExecuteDelayMs = startDelayMs;
}

void StableDebouncer::SetStopDelay(unsigned long stopDelayMs) {
    _debounceStopExecuteDelayMs = stopDelayMs;
}

void StableDebouncer::SetResetCooldown(unsigned long bounceResetCooldownMs) {
    _bounceResetCooldownMs = bounceResetCooldownMs;
}

void StableDebouncer::SetStickyBounce(bool stickyBounce) {
    _isStickyBounce = stickyBounce;
}

void StableDebouncer::Reset() {
    if(_state != Idle)
        _advanceReset();
}