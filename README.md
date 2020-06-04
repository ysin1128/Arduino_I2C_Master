# Arduino_I2C_Master
I2C Master functions for Arduino. 
These have timeout. 

I2C Master write: 
byte i2c_write(byte slave_adr, byte *data, int data_length)

I2C Master read:
byte i2c_read(byte slave_adr, byte *reg_adr, int reg_adr_length, byte *read_data, int read_data_length)
