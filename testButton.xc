#include <xs1.h>
#include <stdio.h>

#define DELAY 100000000;

out port p = XS1_PORT_1A;
in port button = XS1_PORT_32A;

int main(void)
{
    int x;
    int i = 0;
    unsigned state = 0;

    button :> x;

    while(1)
    {
        button when pinsneq(x) :> x;

        state = !state;

        p <: state;

        printf ("%d\n", ++i);
    }

}
