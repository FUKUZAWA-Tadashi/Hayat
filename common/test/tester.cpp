/*  -*- coding: sjis-dos; -*-  */


#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>


const char** HMD_LOADPATH = {
    NULL
};

int main(int argc, char* argv[])
{
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(CppUnit::TestFactoryRegistry::getRegistry().makeTest());
    CppUnit::Outputter* outputter = 
        new CppUnit::CompilerOutputter(&runner.result(),std::cout);
    runner.setOutputter(outputter);
    int result =  runner.run() ? 0 : 1;
    
    return result;
}
