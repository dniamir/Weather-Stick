# include <protocol.h>

Protocol::Protocol(){};

void Protocol::write_register(int address, int reg, int data) {
    return;
}

void Protocol::write_register(int address, int reg, uint8_t data[]) {
    return;
}
   
// void Protocol::read_registers(int address, int reg, int* buffer, int length) {
//     return;
// }

// void Protocol::read_registers(int address, int reg, int* buffer, int length) {
//     return;
// }

int Protocol::read_register(int address, int reg) {
    Serial.println("Using protocol.h");
    return 0;
}

void Protocol::read_register(int address, int reg, int bytes_to_read, int8_t reg_out[]) {
    Serial.println("Using protocol.h");
}
void Protocol::read_register(int address, int reg, int bytes_to_read, uint8_t reg_out[]) {
    Serial.println("Using protocol.h");
}

void Protocol::write_register(char reg){
    Serial.println("Using protocol.h");
}