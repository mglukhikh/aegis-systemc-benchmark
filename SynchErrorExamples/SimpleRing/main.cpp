//*****************************************************************************
//
// In this example there is a ring of nodes, all nodes are equivalent.
// Nodes send messages to each other. A node has unique adress.
// Each node is connected to two nodes. 
// A node can transmit message from left node to right one (one-directed ring).
// If a message is adressed to the node it takes the message and remove it from ring.
//
//    left - fleft -> this -> fright - right -> ...
//    node            node             node
//                   |    |
//                  fin  fout 
//                   |    |
//                  testbench
//
// This variant symbol from ring (left node) has higer priority than symbol from tb_source,
// so no deadlock and no data losses is possible.
//*****************************************************************************

#include "systemc.h"

const int NODE_NUM = 4;
const int MIN_DELAY = 1;
const int MAX_DELAY = 1;
const int WAIT_BEFORE_STOP = 5;
const int FF_SIZE = 3;

int id_gen = 1;
// For statistics
int ss[NODE_NUM+1];    // Send symbols
int sr[NODE_NUM+1];    // receive symbols

sc_clock clk("clk", sc_time(1, SC_NS));

SC_MODULE( node_module )
{
 public:
    int id;		      // Node identifier	

    // For communication with other nodes
    sc_fifo<int> fleft;	      // fifo for left -> this
    sc_fifo_out<int> fright;  // this -> right port, should be connected to 
			      // fifo of the right node

    // For communication with the locale testbench
    sc_fifo<int> fin;         // tb -> node	
    sc_fifo<int> fout;        // node -> tb
    
    
    void set_id(int iid) {
        id = iid;
    }


    // Node process
    void node() {

        wait();
        
        while(true) {
	   // Check if there is a symbol in ring
           int a;
           if (fleft.nb_read(a)) {
	      // Check if the symbol is adressed to this node	
              if (a == id) {
                 // Remove the symbol and send it to the testbench
	         fout.write(a);
                 // Lets consider that node can not send a new symbol 
                 // to ring in this case

	      } else {
 	         // Transmit a symbol to ring
		 fright.write(a);
	      }

           } else {	
	       // Check for symbol from tb 
               int b;
               if (fin.nb_read(b)) {
		   // Send the symbol into ring
                   fright.write(b);	        

               } else {
 		   // Do nothing
	       }		
           }
           wait();    // Only one symbol per clock can be produced
        }
    }

    // Testbench processes
    void tb_source() {
        wait();

        for (int i = 0; i < 3; i++) {
           int a = 1 + rand() % NODE_NUM;
           
	   // Node may not send message to itself   
           if (a != id) { 
              printf("send id = %d to adr = %d\n", id, a);
    	      fin.write(a);
	      ss[a] = ss[a] + 1;  
	   }
	   
	   // Non-deterministic delay
	   wait(MIN_DELAY + rand() % (MAX_DELAY - MIN_DELAY +1));
        }
        // Waiting for all processes finished
        wait(WAIT_BEFORE_STOP);
        

        for (int i = 1; i < NODE_NUM + 1; i++) {
           printf("id = %d, ss = %d, sr = %d\n", i, ss[i], sr[i]);
        }   
    }


    void tb_sink() {

       while(true) {
          int a; 
          fout.read(a);                                 // Error : Violation of infinite main loop repetition in 4 processes
          if (a != id) {
	     printf("wrong symbol in node %d\n", id);	// Error : Violation of no dead code
	  } else {
	     printf("received symbol id = %d\n", a);
	     sr[a] = sr[a] + 1;
	  }
	  wait();
       }
    }


    // Constructor
    SC_CTOR( node_module ): fleft(FF_SIZE), fin(FF_SIZE), fout(FF_SIZE)
    {
        SC_CTHREAD(node, clk);
        SC_CTHREAD(tb_source, clk);
        SC_CTHREAD(tb_sink, clk);
    }
};

int sc_main(int argc, char *argv[])
{
    srand(time(NULL));
    
    // Bind fifo for make a ring
    node_module n1("n1");
    node_module n2("n2");
    node_module n3("n3");
    node_module n4("n4");

    n1.fright(n2.fleft);
    n2.fright(n3.fleft);
    n3.fright(n4.fleft);
    n4.fright(n1.fleft);
    
    n1.set_id(1);
    n2.set_id(2);
    n3.set_id(3);
    n4.set_id(4);


    for (int i = 1; i < NODE_NUM+1; i++) {
       sr[i]= 0; ss[i]= 0;
    }

    sc_start();
}