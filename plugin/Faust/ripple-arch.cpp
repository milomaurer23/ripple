/* Architecture file for scripts/faust.sh. Wraps the Faust base classes and the generated
   RippleDSP class in a namespace: Faust's global `dsp` class would otherwise collide with
   JUCE's `juce::dsp` namespace once JuceHeader.h has done `using namespace juce`. */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <math.h>

// Pulled in here, outside the namespace, so the copy inlined from faust/dsp/dsp.h is a no-op.
#if defined (__SSE__)
#include <xmmintrin.h>
#endif

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

namespace ripplefaust
{
#include "faust/gui/meta.h"
#include "faust/gui/UI.h"
#include "faust/dsp/dsp.h"

<<includeIntrinsic>>

<<includeclass>>

} // namespace ripplefaust
