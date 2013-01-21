//***************************************************************************** 
//
// In this example two dut` processes interact between each other
//
// Bug: ff_1_3 and ff_3_2 will be overflown due to proc_3 duplicating input data, ff_1_2 will be empty,
// it leads to deadlock (proc_2 is waiting for item in ff_1_2, proc_3 is waiting for empty cell in ff_3_2,
// proc_1 is waiting for empty cell in ff_1_3)
//
//***************************************************************************** 

#include "systemc.h"


sc_fifo<int> ain, aout;

SC_MODULE( dut_module )
{
public:
    sc_time clk_period;
    sc_clock clk;

    sc_fifo_in<int> fin;
    sc_fifo_out<int> fout; 

    sc_signal<bool> in_valid, out_valid;
    sc_signal<int> in, out;
    
    // The fifo`s for interconnectint dut_master and mult_accelerator processes
    sc_fifo<int> ff_1_2; // from 1 to 2
    sc_fifo<int> ff_1_3; // from 1 to 3
    sc_fifo<int> ff_3_2; // from 3 to 2

    // tb -> in_wrapper -> dut
    void in_wrapper() {
        int a = 0;
        in_valid.write(false);
        wait();
        
        while (true) {
            if(fin.nb_read(a)) {
                in_valid.write(true);
                in.write(a);
  	        printf("in wrapper write value a=%d\n", a);                            
            }
            wait();
            in_valid.write(false);
            wait();
        }
    }
    
    void proc_1() {
        wait(SC_ZERO_TIME);
        while(true) {
            // Wait for input signal ready
            bool in_ready = in_valid.read();
            if (!in_ready) {
                wait();
                continue;
            }
            int a = in.read();
            wait();
            
            printf("Process #1 reads number a = %d\n",a);
            ff_1_3.write(a);                              // Error: violation of infinite main loop repetition
            ff_1_2.write(a);
            
            wait();
        }
    }
    
    void proc_2() {
        out_valid.write(false);
        wait();
        
        while(true) {
            int a,b;
            ff_1_2.read(a);                               // Error: violation of infinite main loop repetition
            ff_3_2.read(b);
            int res = a+b;
            printf("Process #2 transfer %d and %d to %d\n",a,b,res);
            
            out.write(res);
            out_valid.write(true);
            wait();
            
            out_valid.write(false);
            wait();
        }
    }
    
    void proc_3() {
        while(true) {
            int a;
            ff_1_3.read(a);
            printf("Process #3 reads %d\n",a);
            int res1=2*a;
            printf("Process #3 transfer %d to %d\n",a,res1);
            ff_3_2.write(res1);
            wait();
            int res2=3*a;
            printf("Process #3 transfer %d to %d\n",a,res2);
            ff_3_2.write(res2);                         // Error: violation of infinite main loop repetition
            wait();                                            
        }
    }
    
    // dut -> out_wrapper -> tb
    void out_wrapper() {
        wait();
        while (true) {
            if(out_valid.read()) {
                int a = out.read();
                fout.nb_write(a);
                printf("out wrapper read value a=%d\n", a);
            }
            wait();
        }
    }


    // Constructor
    SC_CTOR( dut_module ): clk_period(1, SC_FS), clk("clk", clk_period), 
                         in_valid(), out_valid(), in(), out()
    {
        SC_CTHREAD(in_wrapper, clk);
        SC_CTHREAD(out_wrapper, clk);
        SC_CTHREAD(proc_1, clk);
        SC_CTHREAD(proc_2, clk);
        SC_CTHREAD(proc_3, clk);
    }
};

SC_MODULE ( tb_module ) 
{

    sc_fifo_in<int> fin;
    sc_fifo_out<int> fout; 
    
    void testbench() {
        for (int i = 0; i < 60; i++) {
            // Factorial for random value
            int j = random() % 10;
	    fout.write(j);
	    
	    int fi;
	    fin.read(fi);                            // Error: violation of finite thread termination

	    printf("factorial for %d is %d\n",j,fi);
        }

        sc_stop();
    }
    
    SC_CTOR (tb_module) {
        SC_THREAD(testbench);
    }
};


int sc_main(int argc, char *argv[])
{
    dut_module dut("m1");
    tb_module tb("m2");
    
    // Bound to testbench
    tb.fout(ain);
    tb.fin(aout);
    // Bound to wrappers
    dut.fin(ain);
    dut.fout(aout);

    sc_start();
}