//*****************************************************************************
//
//  Dining philosophers example
//
//*****************************************************************************

#include "systemc.h"
#include <math.h>

// Philosopher number
const int PHN = 10;

// Forks usage flags
int forks[PHN];

SC_MODULE( my_module )
{

    sc_clock clk;

    // Event for philosopher takes off a fork
    sc_event fork_takeoff;

    void Eating() {
        wait(rand()%3 + 1);
    }

    void Thinking() {
        wait(rand()%3 + 1);
    }

    // Philosopher actions
    void philosopher (int id) {
        while (true) {
            Thinking();

            // Try to take left fork
            while (forks[id]) {                      // Data race error with lines 43, 48 and 53
                wait(fork_takeoff);
            }

            // Take left fork
            forks[id] = 1;                           
            // Right fork is taken next clock
            wait();

            // Try to take right fork
            while (forks[(id+1)%PHN]) {
                wait(fork_takeoff);
            }

            // Take right fork
            forks[(id+1)%PHN] = 1;

            Eating();

            // Take off both forks
            forks[id] = 0;
            forks[(id+1)%PHN] = 0;

            fork_takeoff.notify(SC_ZERO_TIME);
            wait();
        }
    }

    // Process(es)
    void phil1() {
        int id = 0;
        philosopher(id);
    }
    void phil2() {
        int id = 1;
        philosopher(id);
    }
    void phil3() {
        int id = 2;
        philosopher(id);
    }
    void phil4() {
        int id = 3;
        philosopher(id);
    }
    void phil5() {
        int id = 4;
        philosopher(id);
    }
    void phil6() {
        int id = 5;
        philosopher(id);
    }
    void phil7() {
        int id = 6;
        philosopher(id);
    }
    void phil8() {
        int id = 7;
        philosopher(id);
    }
    void phil9() {
        int id = 8;
        philosopher(id);
    }
    void phil10() {
        int id = 9;
        philosopher(id);
    }

    // Constructor
    SC_CTOR( my_module ) : clk("clk", sc_time(1, SC_FS))
    {
        // Forks are unused
        int i;
        for (i = 0; i < PHN; i++) {
            forks[i] = 0;
        }

        SC_CTHREAD( phil1, clk );
        SC_CTHREAD( phil2, clk );
        SC_CTHREAD( phil3, clk );
        SC_CTHREAD( phil4, clk );
        SC_CTHREAD( phil5, clk );
        SC_CTHREAD( phil6, clk );
        SC_CTHREAD( phil7, clk );
        SC_CTHREAD( phil8, clk );
        SC_CTHREAD( phil9, clk );
        SC_CTHREAD( phil10, clk );
    }
};

int sc_main(int argc, char *argv[])
{
    my_module m("m");

    sc_start();
}