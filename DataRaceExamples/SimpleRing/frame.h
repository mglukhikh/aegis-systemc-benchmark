
// Class frame

class Frame {
 public:
   int sender;
   int receiver;
   int data[256]; 
   int d_length;

   Frame(int sender, int receiver, int length = 7) {
      this->sender = sender;
      this->receiver = receiver;
      
      d_length = length - 2;
      
      for (int i = 0; i < d_length; i++) {
          data[i] = 0;
      }
   };
   
   // Create frame from buffer
   Frame(int* buffer, int length = 7) {
      sender = buffer[0];
      receiver = buffer[1];
      d_length = length - 2;
      
      for (int i = 0; i < d_length; i++) {
          data[i] = buffer[i + 2];
      }
   }

   void serialize(int* buffer) {
      int* p = buffer;
      *p = sender;
      p++;
      *p = receiver;
      p++;
      
      for (int i = 0; i < d_length; i++) {
         *p = data[i];
         p++;
      }
   }
};
