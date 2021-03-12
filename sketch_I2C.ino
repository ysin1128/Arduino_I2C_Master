// 06/12/2020 Rev. 1.2
// revised a bug in read function

// for Examples
byte byteSC;
byte byteADR;
byte byteREG[2];
byte byteWRITE[16];
byte byteREAD[16];

int led = 13;
int i;
int cnt_led = 0;

// for I2C functions
const int CONST_TIMEOUT = 100;  //Timeout for I2C functions [ms]
const byte SC_SUCCESS = 0xFF;
const byte SC_TIMEOUT = 0x80;
const byte SC_NACK = 0x00;

void setup() {
  // for Examples
  Serial.begin(9600);
  pinMode(led, OUTPUT);

  // for I2C functions
  pinMode(SDA, INPUT);  //disable internal pull-up
  pinMode(SCL, INPUT);  //disable internal pull-up
  //pinMode(SDA, INPUT_PULLUP);  //enable internal pull-up
  //pinMode(SCL, INPUT_PULLUP);  //enable internal pull-up
  
  TWSR &= ~_BV(TWPS1)&~_BV(TWPS0);  //I2C SCL setting
  TWBR = 72;                        //SCL=100kHz @16MHz
}

void loop() {


  // Example: Write 4-bytes (0x01, 0x08, 0x10, 0x1F) to Slave address: 0x24 (7-bit), Reg 0x00-0x03. 
  byteADR = 0x24;
  byteWRITE[0] = 0x00;  //1-byte Reg address
  byteWRITE[1] = 0x01;  //Data for Reg 0x00
  byteWRITE[2] = 0x08;  //Data for Reg 0x01
  byteWRITE[3] = 0x10;  //Data for Reg 0x02
  byteWRITE[4] = 0x1F;  //Data for Reg 0x03

  byteSC = i2c_write(byteADR, byteWRITE, 5);

  if(byteSC == SC_SUCCESS){
    cnt_led = 1;
  }
  else{
    cnt_led = 2;
  }

  for(i=0;i<cnt_led;i++){ // succeeded -> LED blink x1, failed -> LED blink x2 
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(1000);    
  }

  delay(1000);

  // Example: Read 4-bytes data from Slave address: 0x24 (7-bit), Reg 0x04-0x07.
  byteADR = 0x24;
  byteREG[0] = 0x04;  //1-byte Reg address

  byteSC = i2c_read(byteADR, byteREG, 1, byteREAD, 4);

  if(byteSC == SC_SUCCESS){
    Serial.write(byteREAD, 4);  //Serial out 4-bytes data
    cnt_led = 1;
  }
  else{
    cnt_led = 2;
  }

  for(i=0;i<cnt_led;i++){ // succeeded -> LED blink x1, failed -> LED blink x2 
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(1000);    
  }

  delay(1000);
}

byte i2c_write(byte slave_adr, byte *data, int data_length){
  byte status_code;

  status_code = i2c_main(slave_adr, data, data_length);
  i2c_stop();

  return status_code;
}

byte i2c_read(byte slave_adr, byte *reg_adr, int reg_adr_length, byte *read_data, int read_data_length){
  int i_i2c;
  byte status_code;

  status_code = i2c_main(slave_adr, reg_adr, reg_adr_length);

  if(status_code != SC_SUCCESS){
    return status_code;
  }
  
  status_code = i2c_start();

  if(status_code != SC_SUCCESS){
    return status_code;
  }

  status_code = i2c_send_data((slave_adr<<1) + 1);
    
  if(status_code != SC_SUCCESS){
    return status_code;
  }

  for(i_i2c=0;i_i2c<read_data_length;i_i2c++){
    if(i_i2c == (read_data_length -1)){
      read_data[i_i2c] = i2c_get_data(false);      
    }
    else{
      read_data[i_i2c] = i2c_get_data(true);            
    }
  }

  i2c_stop();
  return SC_SUCCESS;
}

byte i2c_main(byte slave_adr, byte *data, int data_length){
  int i_i2c;
  byte status_code;

  status_code = i2c_start();

  if(status_code != SC_SUCCESS){
    return status_code;
  }

  status_code = i2c_send_data(slave_adr<<1);
    
  if(status_code != SC_SUCCESS){
    return status_code;
  }

  for(i_i2c=0;i_i2c<data_length;i_i2c++){
    status_code = i2c_send_data(data[i_i2c]);

    if(status_code != SC_SUCCESS){
      return status_code;
    }
  }

  return SC_SUCCESS;
}

byte i2c_start(){
  TWCR = _BV(TWINT)|_BV(TWSTA)|_BV(TWEN);

  return i2c_get_status_code(CONST_TIMEOUT);
}

void i2c_stop(){
  TWCR = _BV(TWINT)|_BV(TWSTO)|_BV(TWEN);
}

byte i2c_send_data(byte data){
  TWDR = data;
  TWCR = _BV(TWINT)|_BV(TWEN);

  return i2c_get_status_code(CONST_TIMEOUT);
}

byte i2c_get_data(bool repeat){
  byte status_code;

  if(repeat){
    TWCR = _BV(TWEA)|_BV(TWINT)|_BV(TWEN);
  }
  else{
    TWCR = _BV(TWINT)|_BV(TWEN);    
  }

  if(i2c_get_status_code(CONST_TIMEOUT) == SC_SUCCESS){
    return TWDR;
  }
  else{
    return 0xFF;
  }
}

byte i2c_get_status_code(int timeout){
  byte status_code;
  int cnt_timeout = 0;

  while((TWCR & _BV(TWINT)) == 0){
    if(cnt_timeout == timeout){
      return SC_TIMEOUT;
    }

    cnt_timeout++;
    
    delayMicroseconds(100);  
  }

  status_code = i2c_status_code();

  if(status_code != SC_SUCCESS){
    i2c_stop();
  }

  return status_code;
}

byte i2c_status_code(){
  switch(TWSR & 0xF8){
    case 0x08:  //Stat condition
      return SC_SUCCESS;
    case 0x10:  //Repeted Start condition
      return SC_SUCCESS;
    case 0x18:  //Slave address + W ACK
      return SC_SUCCESS;
    case 0x20:  //Slave address + W NACK
      return SC_NACK;
    case 0x28:  //Data send ACK
      return SC_SUCCESS;
    case 0x30:  //Data send NACK
      return SC_NACK;
    case 0x40:  //Slave address + R ACK
      return SC_SUCCESS;
    case 0x48:  //Slave address + R NACK
      return SC_NACK;
    case 0x50:  //Data read ACK
      return SC_SUCCESS;
    case 0x58:  //Data read NACK
      return SC_SUCCESS;
    default:
      return SC_TIMEOUT;
  }
}
