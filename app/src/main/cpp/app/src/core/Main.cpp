#include <exception>

#include <android_native_app_glue.h>

#include "core/App.h"

using namespace android_slam;

extern "C"
void android_main(android_app* state)
{
    try
    {
        App(state).run();
    }
    catch (std::exception& e)
    {

    }
}