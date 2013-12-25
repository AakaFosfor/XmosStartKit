#include <xs1.h>
#include <stdio.h>

#define DELAY 100000000

out port p = XS1_PORT_1A;
in port button = XS1_PORT_32A;

int main(void)
{
    timer t;

    unsigned state = 1, time;

    t :> time;

    printf("%d\n",time);

    while (1)
    {
        p <: state;
        q <: !state;

        time += DELAY;
        printf("%d\n",time);

        t when timerafter(time) :> void;

        state = !state;
    }
}
