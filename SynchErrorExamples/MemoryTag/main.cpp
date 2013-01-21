//*****************************************************************************
//
// Diamond: 
//       mem
// prod       cons
//       tag
// producer sends addresses to mem & tag
// tag transforms address into tag with some fixed delay
// mem reads data from this address with some random delay
// consumer receives tag, and then data; if data comes first, then it will be missed
//
//*****************************************************************************

#include "systemc.h"

sc_fifo<int> ain, aout;

SC_MODULE( dut_module )
{
public:
    sc_fifo_in<int> fin;
    sc_fifo_out<int> fout; 
    sc_in_clk clk;

    sc_signal<int> in_valid, out_valid, data_valid;
    sc_signal<int> in, out, data; // data & data_valid are used for memory & consumer communication
    
    // The fifo`s for interconnectint dut_master and mult_accelerator processes
    sc_fifo<int> addr;  // addr: prod -> mem
    sc_fifo<int> addr1; // addr1:prod -> tag
    sc_fifo<int> tags;  // tags: tag  -> cons
    

    // tb -> in_wrapper -> dut
    void in_wrapper() {
        int a = 0;
        in_valid.write(0);
        in.write(0);
        wait();
        
        while (true) {
            if(fin.nb_read(a)) {
                in_valid.write(1);
                in.write(a);
  	        printf("in wrapper write value a=%d\n", a);                            
            }
            wait();
            in_valid.write(0);
            wait();
        }
    }
    
    void prod() {
        wait();
        while(true) {
            // Wait for input signal ready
            int in_ready = in_valid.read();
            if (in_ready == 0) {
                wait();
                continue;
            }
            int a = in.read();
            wait();
            
            printf("Producer: reading address a = %d\n",a);
            addr.write(a);
            addr1.write(a);
            
            wait();
        }
    }
    
    void cons() {
        out_valid.write(0);
        out.write(0);
        wait();
        
        while(true) {                              // Error: violation of infinite main loop repetition
            int a,res;
            tags.read(a);
            if (data_valid.read()==1) {
               res = data.read();
               printf("Consumer: %d with tag %d\n",res,a);
               
               out.write(res);
               out_valid.write(1);
               wait();
               
               out_valid.write(0);
            }
            wait();
        }
    }
    
    void tag() {
        wait();
        while(true) {                             // Error: violation of infinite main loop repetition
            int a;
            addr1.read(a);
            // Delay
            wait(7);
            int res =  a ^ 0xff;
            printf("Tag: input %d, output %d\n",a,res);
            tags.write(res);
            wait();
        }
    }
    
    void mem() {
        int d[] = { 97, 45, 23, 57, 12, 0, 74, 29, 33, 66 };
        data_valid.write(0);
        data.write(0);
        wait();
        while(true) {                           // Error: violation of infinite main loop repetition
            int a;
            addr.read(a);
            // Delay
            // If 7 is changed to 6, it leads to deadlock
            wait(7+random()%2);
            int res = d[a];
            printf("Memory: request %d, data %d\n",a,res);
            data.write(res);
            data_valid.write(1);
            wait();
            
            data_valid.write(0);
            wait();
        }
    }
    
    // dut -> out_wrapper -> tb
    void out_wrapper() {
        wait();
        while (true) {
            if(out_valid.read() == 1) {
                int a = out.read();
                fout.nb_write(a);
                printf("out wrapper read value a=%d\n", a);
            }
            wait();
        }
    }


    // Constructor
    SC_CTOR( dut_module ): in_valid(), out_valid(), in(), out()
    {
        SC_CTHREAD(in_wrapper, clk.pos());
        SC_CTHREAD(out_wrapper, clk.pos());
        SC_CTHREAD(prod, clk.pos());
        SC_CTHREAD(cons, clk.pos());
        SC_CTHREAD(mem, clk.pos());
        SC_CTHREAD(tag, clk.pos());
    }
};

SC_MODULE ( tb_module ) 
{

    sc_fifo_in<int> fin;
    sc_fifo_out<int> fout; 
    sc_in_clk clk;
    
    void testbench() {
        wait();
        for (int i = 0; i < 1; i++) {              // False error: violation of finite thread termination
            // Random address
            int j = random() % 10;
	    fout.write(j);
	    
	    int fi;
	    fin.read(fi);

	    printf("Value from %d is %d\n",j,fi);
	    wait();
        }

        sc_stop();
    }
    
    SC_CTOR (tb_module) {
        SC_CTHREAD(testbench, clk.pos());
    }
};


int sc_main(int argc, char *argv[])
{
    srand((unsigned)time(0));
    dut_module dut("m1");
    tb_module tb("m2");
    sc_clock clk;
    
    // Bound to testbench
    tb.fout(ain);
    tb.fin(aout);
    tb.clk(clk);
    // Bound to wrappers
    dut.fin(ain);
    dut.fout(aout);
    dut.clk(clk);

    sc_start();
}