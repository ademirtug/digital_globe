#pragma once
#include "de2.h"
#include <wx/app.h>
struct test_context {
    test_context() { de2::get_instance().init(false); }
    ~test_context() { auto& engine = de2::get_instance(); engine.shutdown(); if (engine.window) { engine.window->Destroy(); wxYieldIfNeeded(); } }
};
