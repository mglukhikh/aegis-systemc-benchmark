
// Class shared buffer
class Buffer {
 private:
   int buf[128]; 

   int wr_i;  // Current write index
   int rd_i;  // Current read index
   int size;  // Capacity

 public:
   Buffer(int capacity) {
      wr_i = 0;
      rd_i = 0;
      size = capacity;
   }
   
   
   // Add data
   bool add(int n, int* data) {
      int num_elements = (wr_i - rd_i + size) % size;
      if (size - num_elements < n) 
         return false;
      for (int i = 0; i < n; i++) {
         buf[wr_i] = data[i];
         wr_i = (wr_i + 1) % size; 
      }               
   }
   
   // Get a number of symbols from buffer, 
   // if there is not enough symbols return false
   bool get(int n, int* data) {
      int num_elements = (wr_i - rd_i + size) % size;
      if (num_elements >= n) {
         for (int i = 0; i < n; i++) {
            data[i] = buf[(rd_i + i) % size];
         }    
         return true;     
      } else {
         return false;
      }
   }    
   
   // Remove number of symbols from buffer,
   // if there is not enough return false
   bool remove(int n) {
      int num_elements = (wr_i - rd_i + size) % size;
      if (num_elements >= n) {
         rd_i = (rd_i + n) % size;
         return true;     
      } else {
         return false;
      }
   }     
};
