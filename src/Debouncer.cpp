#include "Debouncer.h"

Debouncer::Debouncer() {
  _bounceTimeMs = DefaultBounceTimeMs;
}

Debouncer::Debouncer(unsigned long bounceTimeMs) {
  _bounceTimeMs = bounceTimeMs;
}

bool Debouncer::IsBounced() { return (millis() - _lastBounce) < _bounceTimeMs; }

bool Debouncer::IsPermaBounced() { return _permaBounced; }
    
void Debouncer::Reset() { 
  _lastBounce = 0; 
  _permaBounced = false;
}