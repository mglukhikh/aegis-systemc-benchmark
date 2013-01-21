//
// Node module contains processes of node and processes of devices connected to node 
//

#include "systemc.h"
#include "buffer.h"
#include "frame.h"

// Number of nodes in the ring
static const int NODE_NUM = 5;

SC_MODULE( node_module )
{
    static const int BUF_SIZE = 30;
    static const int FRAME_LENGTH = 7; // Header + data length
//    static const int DATA_LENGTH = FRAME_LENGTH - 2; // Data length

    int id;		  // Node address	

    // For communication with other nodes
    Buffer in_buf;        // Input buffer
    Buffer out_buf;       // Output buffer
    Buffer proc_buf;      // Processing buffer
    Buffer* n_buf;        // Input buffer of the neighbor
    
    // Events
    sc_event proc_frame_event;    
    sc_event trans_frame_event;
    sc_event actuator_event;
    sc_event send_event;    

//=====================================================================================
    // Analysis of a received frame
    // This process get a frame from input buffer, analyze receiver address and
    // notify event for process which write frame into inner buffer or event
    // for process which write frame into output buffer
    void node_input_proc() {
       wait(SC_ZERO_TIME);
       
       while(true) {
          int a[FRAME_LENGTH];             
          // Try to get enough symbols
          if (in_buf.get(FRAME_LENGTH, a)) {
             // Check if there is a symbol in input buffer
             Frame f(a, FRAME_LENGTH);
             if (f.receiver == id) {
                // Notify copy to inner buffer process
                proc_frame_event.notify();
             } else {
                // Notify retranslate process
                trans_frame_event.notify();
             }
             //printf("ip : frame received from %d to %d in %d\n", f.sender, f.receiver, id);
          }     
          
          wait(SC_ZERO_TIME);
       }
    } 
    
//=====================================================================================
    // Copy to inner buffer
    // This process wakes up on event, copies all symbols of analysed frame
    // and removes the frame from imput buffer
    void copy_inner_proc() {

       while(true) {
          wait(proc_frame_event);
          
          int a[FRAME_LENGTH];             
          in_buf.get(FRAME_LENGTH, a);
          in_buf.remove(FRAME_LENGTH);

          // Ignore result of add call for simplification          
          proc_buf.add(FRAME_LENGTH, a);
          
          // Wake up the actuator
          actuator_event.notify();
       }
    } 


//=====================================================================================    
    // Actuator device connected to node - uses information recieved from other nodes 
    void actuator_proc() {

       while(true) {
          
          wait(actuator_event);
          
          int a[FRAME_LENGTH];
          if (proc_buf.get(FRAME_LENGTH, a)) {
             proc_buf.remove(FRAME_LENGTH);
             printf("ap : frame received from %d in %d\n", a[0], a[1]);
          }     
       }
    }
    
//=====================================================================================
    // Sensor device connected to node - provides information for transmit
    void sensor_proc() {
    
        while (true) {
           //for (int i = 0; i < 1; i++) {
              if (rand() % 10 > 7) {
                 int address = (rand() % NODE_NUM)  + 1;
                 if (address == id) {
                    address = (address + 1) % NODE_NUM + 1;
                 }        
                 
                 printf("sp : frame created from %d to %d\n", id, address);

                 int a[FRAME_LENGTH];
                 Frame f(id, address, FRAME_LENGTH);
                 f.serialize(a);
                 out_buf.add(FRAME_LENGTH, a);       
                 
                 send_event.notify();
              }
           //}
           
           wait(SC_ZERO_TIME);
        }
    }

//=====================================================================================
    // Move frame from input buffer to putput buffer
    void trans_proc() {

       while(true) {
          wait(trans_frame_event);
          
          int a[FRAME_LENGTH];             
          in_buf.get(FRAME_LENGTH, a);
          in_buf.remove(FRAME_LENGTH);

          // Ignore result of add call for simplification          
          out_buf.add(FRAME_LENGTH, a);      
          send_event.notify();
       }
    } 

//=====================================================================================
    // node_output_proc
    void node_output_proc() {

       while(true) {
          wait(send_event);
          
          int a[FRAME_LENGTH];             
          
          // Copy all frames from output buffer
          while (out_buf.get(FRAME_LENGTH, a)) {
             // printf("op : frame is sent from %d to %d\n", a[0], a[1]);

              out_buf.remove(FRAME_LENGTH);
              n_buf->add(FRAME_LENGTH, a);      
          }      
       }
    } 

//=====================================================================================

 public:    
    void set_id(int id) {
       this->id = id;
    }

    // Constructor
    SC_CTOR( node_module ) : in_buf(BUF_SIZE), out_buf(BUF_SIZE), proc_buf(BUF_SIZE)  
    {
        SC_THREAD(node_input_proc);
        SC_THREAD(copy_inner_proc);
        SC_THREAD(sensor_proc);
        SC_THREAD(actuator_proc);
        SC_THREAD(trans_proc);
        SC_THREAD(node_output_proc);
    }
};
