//*****************************************************************************
//
//  Producer-consumer example
//
//*****************************************************************************

#include "systemc.h"

SC_MODULE( my_module )
{
    char* s;
    char c;
    sc_event send;
    sc_clock clk;
    
    // Producer
    void producer() {

        wait();      	

        while (*s) {
            c = *s;                 // No data race error
            s++;
            send.notify();	   	
	    wait();         
        };
    };

    // Consumer
    void consumer() {

        while (true) {
            // Waiting for a symbol
            wait(send);     

            char a = c;
            if (a == 'x') {
                break;
            }
        }
    }

    // constructor
    SC_CTOR( my_module ) : clk("clk", sc_time(1, SC_FS))
    {
	s = (char*)"Hello world and exit";
	c = 0;

        SC_CTHREAD(producer, clk); 
        SC_CTHREAD(consumer, clk);
    }
};

int sc_main(int argc, char *argv[])
{
    my_module m("m");

    sc_start();
}