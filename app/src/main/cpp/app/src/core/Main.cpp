#include <exception>

#include <android_native_app_glue.h>

#include "core/App.h"

using namespace android_slam;

/* 由于安卓的main函数为C风格，因此为了屏蔽掉C的形式，采用了main函数仅包含一个异常处理，
 * 最终全部交由App处理的编码形式
 */
extern "C" void android_main(android_app* state)
{
    try
    {
        App(state).run();
    }
    catch (std::exception& e)
    {}
}