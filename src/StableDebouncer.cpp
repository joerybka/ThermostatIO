//
// Created by joerr on 20-Apr-24.
//

#include "StableDebouncer.h"

StableDebouncer::StableDebouncer() : StableDebouncer(DefaultDebounceMilliseconds) { }

StableDebouncer::StableDebouncer(unsigned long bounceMs) {
    _bounceMs = bounceMs;
}

void StableDebouncer::SetStartBounceDelay(unsigned long bounceStartDelayMs) {
    _bounceStartDelayMs = bounceStartDelayMs;
}

void StableDebouncer::SetBounceResetCooldown(unsigned long bounceResetCooldownMs) {
    _bounceResetCooldownMs = bounceResetCooldownMs;
}

void StableDebouncer::SetStickyBounce(bool stickyBounce) {
    _isStickyBounce = stickyBounce;
}

void StableDebouncer::Reset() {
    _reset();
}