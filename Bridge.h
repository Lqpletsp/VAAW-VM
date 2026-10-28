#pragma once
#include <stdbool.h>
typedef enum {
    Constant, 
    VMMemoryType, 
    VMInstructions, 
    VMKeywords, 
} MainByteCodeTypes; 

typedef enum {
    SETI,  
    SETW, 
    SETA, 
    SETC, 
    OUTPUT, 
    DECM, // Declare Memory
    GOTO,
} VM_Instructions; 

typedef enum {
    TRUE, 
    FALSE, 
} VM_Keywords;

typedef enum {
    VVM_INTEGER, 
    VVM_CHAR, 
    VVM_DOUBLE, 
    VVM_BOOLEAN,
} VVM_DataTypes; // VVM -> Virtual Volatile Memory 

typedef enum {
    VM_INTEGER, 
    VM_CHAR, 
    VM_STRING, 
    VM_BOOLEAN, 
    VM_DOUBLE, 
    VM_ADDRESS,
} VM_DataTypes; // VM -> Virtual Machine 

typedef struct {
    VVM_DataTypes dtp; 
    union{
        char data_clt;
        int data_ilt;
        double data_dlt;
        bool data_blt;
    } data;  
} RawMemorySpace;  

typedef struct{
    // if not constant use empty_lt else others accordingly
    union{
        char data_clt;
        char *data_slt; 
        int data_ilt;
        double data_dlt;
        bool data_blt;
    } lt; 
    MainByteCodeTypes main_type; 
    union{
        VM_DataTypes vvmdtp; 
        VM_Instructions vinst; 
        VM_Keywords vkey; 
    } sub_type;  
} Code; 

typedef struct{
    VVM_DataTypes dtp; 
    // if INTEGER, DOUBLE, or CHAR then use address else addresses
    bool immutable; 
    union{
        int address; 
        int *addresses; 
    } storage; 
} Variable; 
