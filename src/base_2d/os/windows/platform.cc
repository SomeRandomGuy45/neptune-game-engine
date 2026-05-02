#include <os/platform.h>
#include <iostream>

// TODO
namespace neptune {

    void debugFunction_01() {
        std::cout << "Hello world!" << std::endl;
    }
    
    void fixActivationPolicyFunc() {
        std::cout << "Not needed on windows!" << std::endl;
    }

    char* getFileFromPicker() {
        return nullptr;
    }

}