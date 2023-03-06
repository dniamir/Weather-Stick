# include <Arduino.h>

#ifndef _PROTOCOL_
#define _PROTOCOL_

// An abstract class
class Protocol {   
    // Data members of class
public:
    Protocol(); 

    void write_register(char);

    // ~Protocol();
    // Pure Virtual Function
    virtual void write_register(int address, int reg, int data);
    virtual void write_register(int address, int reg, uint8_t data[]);
    virtual int read_register(int address, int reg);
    virtual void read_register(int address, int reg, int bytes_to_read, int8_t reg_out[]);
    virtual void read_register(int address, int reg, int bytes_to_read, uint8_t reg_out[]);
    
   /* Other members */
}; 

#endif