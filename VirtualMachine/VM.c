#include "VM.h"
#include "../Bridge.h"
#include <stdio.h>
#include <stdlib.h>
static int TotalMemory; 
static RawMemorySpace *VirtualVolatileMemory;  

static int GReg0; 
static int GReg1; 
static int GReg2; 
static int GReg3; 
static int GReg4; 
static int CSP; // Code Stream Pointer

typedef struct{
    int collection[512]; 
    int size; 
    int front; 
    int rear; 
    int capacity; 
} special_queue; 

typedef struct {
    int collection[512];
    int size; 
    int top; 
    int capacity; 
} stack;

static special_queue WBuffer, IBuffer, GBuffer; 
static stack CBuffer; 
typedef struct {
    Code *stream; 
    int capacity; 
    int size; 
} CodeStream; 

static CodeStream ByteCode; 

enum Exceptions{
    FullWBuffer, 
    EmptyWBuffer,
    FullCBuffer,
    EmptyCBuffer,
    FullGBuffer, 
    EmptyGBuffer,
    FullIBuffer,  
    EmptyIBuffer, 
    WBufferOverflow, 
    IBufferOverflow, 
    GBufferOverflow, 
};
static char *ExceptionMessages[] = {
    "Full WBuffer",
    "Empty WBuffer", 
    "Full CBuffer",
    "Empty CBuffer", 
    "Full GBuffer",
    "Empty GBuffer", 
    "Full IBuffer",
    "Empty IBuffer", 
    "Index for WBuffer will lead to out of bounds",
    "Index for IBuffer will lead to out of bounds", 
    "Index for GBuffer will lead to out of bounds", 
};
int InitializeByteCode(){
    ByteCode.capacity = 256; 
    ByteCode.stream = (Code *)malloc(ByteCode.capacity * sizeof(Code)); 
    return (ByteCode.stream == NULL) ? 1 : 0; 
}
int AppendCodeStream(Code code){
    if (ByteCode.size >= ByteCode.capacity - 1){
        ByteCode.capacity *= 2; 
        ByteCode.stream = (Code *)malloc(ByteCode.capacity * sizeof(Code)); 
        if (ByteCode.stream == NULL) return 1; 
    }
    ByteCode.stream[ByteCode.size] = code; 
    ByteCode.size += 1; 
    return 0; 
}
Code *GetByteCodeState(){return ByteCode.stream;}
void InitializeSys(){
    WBuffer.capacity = 512; 
    IBuffer.capacity = 512; 
    GBuffer.capacity = 512; 
    CBuffer.capacity = 512; 
}
void HandleExceptions(){
    // searches for error code in GReg0
    printf("%s", ExceptionMessages[GReg0]);  
    // this module will handle exception messages using OUTPUT function of the VM 
    exit(1);
}
void EnqueueWBuffer(){
    // stores the value of GReg1 in WBuffer 
    if (WBuffer.size >= WBuffer.capacity){
        GReg0 = (int)FullWBuffer; 
        HandleExceptions(); 
    }
    if (WBuffer.rear >= WBuffer.capacity) WBuffer.rear = 0; 
    WBuffer.collection[WBuffer.rear] = GReg1; 
    WBuffer.rear += 1; 
    WBuffer.size += 1; 
}
void DequeueWBuffer(){
    if (WBuffer.size == 0){
        GReg0 = (int)EmptyWBuffer; 
        HandleExceptions(); 
    }
    WBuffer.collection[WBuffer.front] = 0; 
    WBuffer.front += 1; 
    if (WBuffer.front >= WBuffer.capacity) WBuffer.front = 0; 
    WBuffer.size -= 1; 
}
void GetFrontWBuffer(){
    // stores the front value to GReg0
    if (WBuffer.size == 0){
        GReg0 = (int)EmptyWBuffer; 
        HandleExceptions(); 
    }
    GReg0 = WBuffer.collection[WBuffer.front]; 
}
void GetBackWBuffer(){
    if (WBuffer.size == 0){
        GReg0 = (int)EmptyWBuffer;
        HandleExceptions(); 
    }
    GReg0 = WBuffer.collection[WBuffer.rear];
}
void AtWBuffer(){
    // uses the index in GReg1 and stores the data in that index to GReg2 
    if (GReg1 > WBuffer.capacity){
        GReg0 = (int)WBufferOverflow; 
        HandleExceptions(); 
    }
    GReg0 = WBuffer.collection[GReg1]; 
}
void EnqueueGBuffer(){
    // stores the value of GReg1 in WBuffer 
    if (GBuffer.size >= GBuffer.capacity){
        GReg0 = (int)FullWBuffer; 
        HandleExceptions(); 
    }
    if (GBuffer.rear >= GBuffer.capacity) GBuffer.rear = 0; 
    GBuffer.collection[GBuffer.rear] = GReg1; 
    GBuffer.rear += 1; 
    GBuffer.size += 1; 
}
void DequeueGBuffer(){
    if (GBuffer.size == 0){
        GReg0 = (int)EmptyGBuffer; 
        HandleExceptions(); 
    }
    GBuffer.collection[GBuffer.front] = 0; 
    GBuffer.front += 1; 
    if (GBuffer.front >= GBuffer.capacity) GBuffer.front = 0; 
    GBuffer.size -= 1; 
}

void GetFrontGBuffer(){
    // stores the front value to GReg0
    if (GBuffer.size == 0){
        GReg0 = (int)EmptyGBuffer; 
        HandleExceptions(); 
    }
    GReg0 = GBuffer.collection[GBuffer.front]; 
}

void GetBackGBuffer(){
    if (GBuffer.size == 0){
        GReg0 = (int)EmptyGBuffer;
        HandleExceptions(); 
    }
    GReg0 = GBuffer.collection[GBuffer.rear];
}

void AtGBuffer(){
    // uses the index in GReg1 and stores the data in that index to GReg2 
    if (GReg1 > GBuffer.capacity){
        GReg0 = (int)GBufferOverflow; 
        HandleExceptions(); 
    }
    GReg0 = GBuffer.collection[GReg1]; 
}
void EnqueueIBuffer(){
    // stores the value of GReg1 in WBuffer 
    if (IBuffer.size >= IBuffer.capacity){
        GReg0 = (int)FullWBuffer; 
        HandleExceptions(); 
    }
    if (IBuffer.rear >= IBuffer.capacity) IBuffer.rear = 0; 
    IBuffer.collection[IBuffer.rear] = GReg1; 
    IBuffer.rear += 1; 
    IBuffer.size += 1; 
}
void DequeueIBuffer(){
    if (IBuffer.size == 0){
        GReg0 = (int)EmptyIBuffer; 
        HandleExceptions(); 
    }
    IBuffer.collection[IBuffer.front] = 0; 
    IBuffer.front += 1; 
    if (IBuffer.front >= IBuffer.capacity) IBuffer.front = 0; 
    IBuffer.size -= 1; 
}

void GetFrontIBuffer(){
    // stores the front value to GReg0
    if (IBuffer.size == 0){
        GReg0 = (int)EmptyIBuffer; 
        HandleExceptions(); 
    }
    GReg0 = IBuffer.collection[IBuffer.front]; 
}

void GetBackIBuffer(){
    if (IBuffer.size == 0){
        GReg0 = (int)EmptyIBuffer;
        HandleExceptions(); 
    }
    GReg0 = IBuffer.collection[IBuffer.rear];
}

void AtIBuffer(){
    // uses the index in GReg1 and stores the data in that index to GReg2 
    if (GReg1 > IBuffer.capacity){
        GReg0 = (int)IBufferOverflow; 
        HandleExceptions(); 
    }
    GReg0 = IBuffer.collection[GReg1]; 
}

void PushCBuffer(){
    // Pushes the value of GReg1
    if (CBuffer.size >= CBuffer.capacity){
        GReg0 = (int)FullCBuffer; 
        HandleExceptions(); 
    }
    CBuffer.collection[CBuffer.top] = GReg1; 
    CBuffer.top += 1; 
    CBuffer.size += 1;
}
void TopCBuffer(){
    if (CBuffer.size == 0){
        GReg0 = (int)EmptyCBuffer; 
        HandleExceptions(); 
    }
    GReg0 = CBuffer.collection[CBuffer.top]; 
}
void PopCBuffer(){
    if (CBuffer.size == 0){
        GReg0 = (int)EmptyCBuffer; 
        HandleExceptions(); 
    }    
    CBuffer.collection[CBuffer.top] = 0; 
    CBuffer.top -= 1; 
    CBuffer.size -=1; 
}
void ExecuteByteCode(){
}