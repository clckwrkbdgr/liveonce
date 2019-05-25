// winport.cpp : Defines the entry point for the console application.
//

/* -- Include the precompiled libraries -- */
#ifdef WIN32
#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")
#endif

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

