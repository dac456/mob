#include "Root.h"

int main(int argc, char* argv[])
{

    mob::root mob;
    mob.mob_init(argc, argv);
    
    for(;;);
    
    mob.mob_kill();

}