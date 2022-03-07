/**
FileName:   main.cc
CreateDate: 20/1/2022
Author:     ticks
Email:      2938384958@qq.com
Des:        
*/
#include <iostream>
#include "base/Log.h"

using namespace SinBack;

int main()
{
    Log::Logger log("SinBack.log");
    log.info("Hello, I am info {}", "Ticks");
    Int i = 0;
    for (; i < 1000; ++i){
        log.warn("this is {} log message!", i);
    }

    fmt::print("Hello World {}", 12);
    return 0;
}
