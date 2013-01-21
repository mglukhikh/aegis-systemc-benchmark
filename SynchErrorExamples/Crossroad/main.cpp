//*****************************************************************************
//
//  Crossroad example
//
//*****************************************************************************


#include "systemc.h"
#include <math.h>

enum direction {
forward = 0,
left = 1,
right = 2,
none = 3 // no car
};

SC_MODULE( crossroads )
{

    sc_clock clk;
    // Directions of cars from all 4 sides of crossroads
    direction x[4];
    

    // Main function for specific direction
    void drive (int id) {
        while (true) {                                // Error : Violation of infinite main loop repetition in four processes
    	    // Generate direction
    	    x[id] = (direction) (random() % 4);       // Error : Data race in four processes
    	    wait();

    	    switch(x[id]) {
    	    case forward:
    		move_forward(id);
    		break;
    	    case left:
    		move_left(id);
    		break;
    	    case right:
    	    case none:
    		; // nothing to do
    	    }
        }
    }

    // Forward direction handler
    void move_forward(int id) {
	int right_index = (id - 1 + 4) % 4;

	// waiting for none right car
	while (x[right_index] != none) {
	    wait();
	}
    }

    // Left direction handler
    void move_left(int id) {
	int right_index = (id - 1 + 4) % 4;
	int forward_index = (id + 2) % 4;

	// waiting for none right car and none forward car
	while (x[right_index] != none || x[forward_index] != none) {
	    wait();
	}
    }

    // Process(es)
    void north() {
        drive(0);
    }
    void east() {
	drive(1);
    }
    void south() {
        drive(2);
    }
    void west() {
	drive(3);
    }

    // Constructor
    SC_CTOR( crossroads )
    {
        SC_CTHREAD(north, clk);
        SC_CTHREAD(east, clk);
        SC_CTHREAD(south, clk);
        SC_CTHREAD(west, clk);
    }
};

int sc_main(int argc, char *argv[])
{
    crossroads c("c");

    sc_start();
}