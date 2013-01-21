//*****************************************************************************
//
// This is a example of dut which implement a simple FSM.
// 
//*****************************************************************************

#include "systemc.h"

sc_fifo<int> ain, aout;

const int M_INIT = 1;
const int M_CALC = 2;
const int M_GET = 3;

SC_MODULE( dut_module )
{
 public:
    sc_in_clk clk;
    sc_fifo_in<int> fin;
    sc_fifo_out<int> fout; 
    sc_fifo<int> m_in;
    sc_fifo<int> m_out;
    sc_signal<int> init;
    
    // Factorial calculation imitates various delay
    int factorial(int v) {
        int res = 1;
	for (int i = 1; i <= v; i++) {
	   if (init == 1) break;
	   
	   res = res * i; 
	   wait();	
	}
	return res;
    }
    
    // DUT process 1
    void fact_proc() {
        wait();

        while (true) {
           // Waiting for INIT command
           if (init != 1) {
              wait();                     // Error: violation of infinite main loop repetition
              continue;
           }     

           // Read value
           int a;
           m_in.read(a);	     
           wait();
           
           // Calculate factorial
           int b = factorial(a);     
           m_out.write(b);           
           wait();	
        }
    }

    // DUT process 2
    void dut_proc() {
        init = 0;             		     
        wait();
        
        // Main cycle
        while (true) {
           int a;
           // Read command
           fin.read(a);                      
           wait();
           
           // DUT FSM
           if (a == M_INIT) {
              // Init  
              init = 1;                      
              wait();
              
           } else if (a == M_CALC) {
              // Calc factorial of random value
              m_in.nb_write(rand() % 5);     
              wait();
              
           } else if (a == M_GET) {
              // Get a result
              int b;
              m_out.read(b);              // Error: violation of infinite main loop repetition   
              fout.write(b);
              wait();
           
           } else {
              // Unknown command
              continue; 
           }
        }
    }    
    
    // Constructor
    SC_CTOR( dut_module ) 
    {
        SC_CTHREAD(fact_proc, clk.pos());
        SC_CTHREAD(dut_proc, clk.pos());
    }
};

SC_MODULE ( tb_module ) 
{

    sc_in_clk clk;
    sc_fifo_in<int> fin;
    sc_fifo_out<int> fout; 
    
    void testbench() {

       // First calc             
       if (rand()) {
          fout.write(M_INIT);		// Possible send of INIT command
       }

       fout.write(M_CALC);
       wait();

       fout.write(M_GET);  
       wait();

       wait(100);
	                
       sc_stop();
    }                                  
    
    SC_CTOR (tb_module) {
        SC_CTHREAD(testbench, clk.pos());    
    }
};

int sc_main(int argc, char *argv[])
{
    sc_report_handler::set_actions("/IEEE_Std_1666/deprecated", SC_DO_NOTHING);
    srand(time(NULL));
    
    sc_clock clk("clk", sc_time(1, SC_NS));
    dut_module dut("m1");
    dut.clk(clk);
    tb_module tb("m2");
    tb.clk(clk);
    
    // Bind to testbench
    tb.fout(ain);    tb.fin(aout);
    // Bind to wrappers
    dut.fin(ain);
    dut.fout(aout);
    
    sc_start();
}