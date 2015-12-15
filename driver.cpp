// header files ===============================================================
#include <iostream>
#include "ossim.h"

using namespace std;

// class definitions
// none

// global variables/constants =================================================
// none

// function prototypes ========================================================
// none

// main driver ================================================================
int main(int argc, char** argv)
{
    // declarations/initializations
    ossim os;

    // check if the usage is correct
    if ( argc != 2 )
    {
        printf("Usage: %s [filepath]\n", argv[0]);
        return -1;
    }

    // if the OS is configured correctly , start simulation
    if(!os.init(argv[1]))
    {
        return 1;
    }

    os.start();
    return 0;
}
// supporting function implementations ========================================
// none
