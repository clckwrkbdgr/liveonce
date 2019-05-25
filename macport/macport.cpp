// linuxport.cpp : Defines the entry point for the console application.
//

#ifdef main
#undef main
#endif

#define main real_main

#include "../main.cpp"

#undef main

int 
main(int argc, char **argv)
{
    real_main();
    return 0;
}

