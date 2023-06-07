byte data_out[10];
volatile byte data_in[10];
volatile int pos;
volatile boolean data_ready;

void setup (void)
{
  pinMode(7, OUTPUT); pinMode(8, OUTPUT); pinMode(9, OUTPUT);  
  SPCR = (1<<SPE)|(1<<SPIE);     // Enable SPI + Interruption

  asm
  (
  "in  r30,0x2D  \n\t"           // Clear SPSR
  "in  r30,0x2E"                 // Clear SPDR
  );
  
  asm
  (
  "sei"                          // Enable interrupt
  );
  
  Serial.begin(115200);
}

ISR (SPI_STC_vect)               // Interruption if SPI packet is ready
{
  if (pos < 9)
  {
    data_in[pos] = SPDR;
    pos++;
  }
  else
  {
    data_in[pos] = SPDR;
    pos = 0;
    data_ready = true;
  }
}

void loop (void)                 // Main routine
{
  if (data_ready)
  {
    copyData();
    writeData(0);
    writeData(1);
    printData();
    data_ready = false;
  } 
}

void copyData(void)              // Copy input bytes to output bytes
{
  if (data_in[0] == B10001101)
  {
    data_out[0] = (B00101101);   // Channel(3)+0+Gain(4)
    data_out[1] = data_in[1];    // Volume(8)
    data_out[2] = (B11000000);   // Bass(4)+0000
    data_out[3] = (B11000000);   // Treb(4)+Fade(4)
    data_out[4] = (B10010000);   // Ch-sel(2)+Fader(1)+Loud(1)+Zerocross(4)
    
    data_out[5] = (B00101101);
    data_out[6] = data_out[1];
    data_out[7] = (B11000000);
    data_out[8] = (B11000000);
    data_out[9] = (B01010000);
  }
  else
  {
    for(uint8_t i=0; i<10; i++)
    {
      data_out[i] = data_in[i];
    }
  }
}

void writeData(int output_data)  // Write packet to CCB bus
{
  writeAddress();
  switch (output_data)
  {
    case 0:
      for(uint8_t i=0; i<5; i++)
      {
        writeByte(data_out[i]);
      }
      break;
    case 1:
      for(uint8_t i=5; i<10; i++) {
        writeByte(data_out[i]);
      }
      break;
  }
  writeNibble();
  digitalWrite(9, 0);
}

void writeAddress(void)          // Write the first byte to CCB bus
{
  digitalWrite(7, 0); digitalWrite(9, 1);
  digitalWrite(9, 0); delayMicroseconds(2);
  writeByte(B10000001);
  digitalWrite(9, 1); delayMicroseconds(2);
} 
  
void writeNibble(void)           // Write last 4 bits to CCB bus
{
  for (uint8_t i=0; i<4; i++)
 { 
    digitalWrite(7, 0);
    digitalWrite(8, 0); delayMicroseconds(2);
    digitalWrite(7, 1); delayMicroseconds(2);
    digitalWrite(7, 0);
  }
}

void writeByte(byte data)        // Send byte to CCB bus
{
  for(int8_t i=7; i>=0; i--)
  {
    digitalWrite(7, 0);
    digitalWrite(8, bitRead(data, i)); delayMicroseconds(2);
    digitalWrite(7, 1); delayMicroseconds(2);
    digitalWrite(7, 0);
  }
}

void printByte(byte data)        // Send byte to serial
{
  for(int8_t i=7; i>=0; i--)
  {
    uint8_t tempbit = bitRead(data, i);
    Serial.print(tempbit);
  }
  Serial.print(" ");
}

void printData(void)             // Send packet to serial
{
  for(uint8_t i=0; i<10; i++)
  {
    printByte(data_out[i]);
  }
  Serial.println();
}
