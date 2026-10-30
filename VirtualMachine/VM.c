#include "VM.h"
#include "../Bridge.h"
#include <stdlib.h>
#include <unistd.h>
static int TotalMemory;
static RawMemorySpace *VirtualVolatileMemory;

static int GReg0;
static int GReg1;
static int GReg2;
static int GReg3;
static int GReg4;
static int CSP = -1; // Code Stream Pointer
typedef struct {
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
typedef struct {
  RawMemorySpace collection[256];
  int free_left;
  int next_addr;
  int capacity;
} GCache;
static special_queue WBuffer, IBuffer, GCache0;
static stack CBuffer;
typedef struct {
  Code *stream;
  int capacity;
  int size;
} CodeStream;

static CodeStream ByteCode;
static Code *CC; // Current code

static void HandleExceptions();
enum Exceptions {
  FullCBuffer,
  EmptyCBuffer,
  GarbageArgForDECM,
  MemoryAlreadyDeclared,
  MemoryInitializationFailed,
  UnknownInstruction,
  NonIntConstantForCSP,
  GarbageArgForGOTO,
  GarbageArgForSETA,
  AttemptedOutOfBoundsCSP
};
static char *ExceptionMessages[] = {
    "Full CBuffer",
    "Empty CBuffer",
    "Argument for DECM instruction not recognized",
    "Attempted to declare memory more than once",
    "Memory initialization failed",
    "Unrecognized instruction",
    "Non integer constant given for CSP",
    "Argument for GOTO instruction not recognized",
    "Argument for SETA instruction not recognized",
    "CSP changed is out-of-bounds for the current bytecode"};
int InitializeByteCode() {
  ByteCode.capacity = 256;
  ByteCode.stream = (Code *)malloc(ByteCode.capacity * sizeof(Code));
  return (ByteCode.stream == NULL) ? 1 : 0;
}
int AppendCodeStream(Code code) {
  if (ByteCode.size >= ByteCode.capacity - 1) {
    ByteCode.capacity *= 2;
    ByteCode.stream = (Code *)malloc(ByteCode.capacity * sizeof(Code));
    if (ByteCode.stream == NULL)
      return 1;
  }
  ByteCode.stream[ByteCode.size] = code;
  ByteCode.size += 1;
  return 0;
}
bool GetByteCodeState() { return (ByteCode.stream == NULL) ? false : true; }
void InitializeSys() {
  GCache0.capacity = 512;
  WBuffer.capacity = 512;
  IBuffer.capacity = 512;
  CBuffer.capacity = 512;
}

static void DequeueGCache0() {
  if (GCache0.size == 0) {
    GReg0 = -1;
  }
  GCache0.collection[GCache0.front] = 0;
  GCache0.front += 1;
  if (GCache0.front >= GCache0.capacity)
    GCache0.front = 0;
  GCache0.size -= 1;
}
static void EnqueueGCache0() {
  // stores the value of GReg1 in GCache0
  if (GCache0.size >= GCache0.capacity) {
    DequeueGCache0();
  }
  if (GCache0.rear >= GCache0.capacity)
    GCache0.rear = 0;
  GCache0.collection[GCache0.rear] = GReg1;
  GCache0.rear += 1;
  GCache0.size += 1;
}
static void GetFrontGCache0() {
  // stores the front value to GReg0
  if (GCache0.size == 0) {
    GReg0 = -1;
  }
  GReg0 = GCache0.collection[GCache0.front];
}

static void GetBackGCache0() {
  if (GCache0.size == 0) {
    GReg0 = -1;
  }
  GReg0 = GCache0.collection[GCache0.rear];
}

static void AtGCache0() {
  // uses the index in GReg1 and stores the data in that index to GReg2
  if (GReg1 > GCache0.capacity) {
    GReg0 = -1;
  }
  GReg0 = GCache0.collection[GReg1];
}
static void PushCBuffer() {
  // Pushes the value of GReg1
  if (CBuffer.size >= CBuffer.capacity) {
    GReg0 = (int)FullCBuffer;
    HandleExceptions();
  }
  CBuffer.collection[CBuffer.top] = GReg1;
  CBuffer.top += 1;
  CBuffer.size += 1;
}
static void TopCBuffer() {
  if (CBuffer.size == 0) {
    GReg0 = (int)EmptyCBuffer;
    HandleExceptions();
  }
  GReg0 = CBuffer.collection[CBuffer.top];
}
static void PopCBuffer() {
  if (CBuffer.size == 0) {
    GReg0 = (int)EmptyCBuffer;
    HandleExceptions();
  }
  CBuffer.collection[CBuffer.top] = 0;
  CBuffer.top -= 1;
  CBuffer.size -= 1;
}

static void IncrementCSP() {
  ++CSP;
  CC = &ByteCode.stream[CSP];
}
static void SET_INT_IN_CBUFFER() {
  // helper function
  GReg2 = CC->lt.data_ilt;
  GReg1 = -1;
  PushCBuffer();
  GReg1 = (GReg2 < 0) ? (int)'-' : (int)'+';
  PushCBuffer();
  do {
    GReg3 = GReg2 % 10;
    GReg2 /= 10;
    GReg1 = (int)(char)(GReg3 + '0');
    PushCBuffer();
  } while (GReg2 < 10);
}

static void DECLARE_MEMORY_inst() {
  if (TotalMemory > 0) {
    GReg0 = (int)MemoryAlreadyDeclared;
    HandleExceptions();
  }
  IncrementCSP();
  if (CC->main_type != Constant) {
    GReg0 = (int)GarbageArgForDECM;
    HandleExceptions();
  }
  if (CC->sub_type.vvmdtp != VM_INTEGER) {
    GReg0 = (int)GarbageArgForDECM;
    HandleExceptions();
  }
  GReg0 = CC->lt.data_ilt;
  TotalMemory = GReg0;
  VirtualVolatileMemory =
      (RawMemorySpace *)malloc(TotalMemory * sizeof(RawMemorySpace));
  if (VirtualVolatileMemory == NULL && CSP != 0) {
    GReg0 = (int)MemoryInitializationFailed;
    HandleExceptions();
  }
}
static void OUTPUT_inst() {
  GReg4 = GReg0;
  while (GReg0 != -1) {
    write(1, &GReg4, 1);
    // arg: 1->stdout, 2-> buffer, 3->number of bytes to read
  }
}

static void HandleExceptions() {
  free(ByteCode.stream);
  free(VirtualVolatileMemory);
  // if IBuffer is full, cannot store the exception message
  IBuffer.front = 0;
  IBuffer.rear = 0;
  IBuffer.size = 0;
  GReg4 = 0;
  // searches for error code in GReg0
  while (ExceptionMessages[GReg0][GReg4] != '\0') {
    GReg1 = (int)ExceptionMessages[GReg0][GReg4];
    ++GReg4;
  }
  GReg1 = -1;
  // -1 is the null terminator for int array (queue) used by OUTPUT_inst
  OUTPUT_inst();
  exit(1);
}
static void SETA_inst() {
  // format: SETA <destination> <source>
  IncrementCSP();
  if (CC->main_type != Constant && CC->sub_type.vvmdtp != VM_ADDRESS) {
    GReg0 = (int)GarbageArgForSETA;
    HandleExceptions();
  }
  GReg0 = CC->lt.data_ilt;
  IncrementCSP();
  switch (CC->sub_type.vvmdtp) {
  case VM_INTEGER:
  case VM_CHAR:
  case VM_BOOLEAN:
  case VM_DOUBLE:
  case VM_ADDRESS:
    VirtualVolatileMemory[GReg0].data =
        VirtualVolatileMemory[CC->lt.data_ilt].data;
    break;
  case VM_STRING:
    GReg1 = 0;
    while (CC->lt.data_slt[GReg1] != '\0') {
      VirtualVolatileMemory[GReg0 + GReg1].data.data_clt =
          CC->lt.data_slt[GReg1];
      ++GReg1;
    }
    VirtualVolatileMemory[GReg0 + GReg1].data.data_clt = 0;
    break;
  }
}
void static GOTO_inst() {
  IncrementCSP();
  switch (CC->main_type) {
  case Constant: {
    switch (CC->sub_type.vvmdtp) {
    case VM_INTEGER:
      if (CC->sub_type.vvmdtp != VM_INTEGER) {
        GReg0 = (int)GarbageArgForGOTO;
        HandleExceptions();
      }
      GReg0 = CC->lt.data_ilt;
      if (GReg0 >= ByteCode.size || GReg0 < 0) {
        GReg0 = (int)AttemptedOutOfBoundsCSP;
        HandleExceptions();
      }
      break;
    case VM_ADDRESS:
      GReg0 = CC->lt.data_ilt;
      if (GReg0 >= ByteCode.size || GReg0 < 0) {
        GReg0 = (int)AttemptedOutOfBoundsCSP;
        HandleExceptions();
      }
      break;
    default:
      GReg0 = (int)GarbageArgForGOTO;
      HandleExceptions();
      break;
    }
    CSP = GReg0;
    CC = &ByteCode.stream[CSP];
  }
  default:
    GReg0 = (int)GarbageArgForGOTO;
    HandleExceptions();
    break;
  }
}
void ExecuteByteCode() {
  IncrementCSP();
  while (CSP < ByteCode.size) {
    if (CC->main_type == VMInstructions) {
      switch (CC->sub_type.vinst) {
      case SETA:
        SETA_inst();
        break;
      case GOTO:
        GOTO_inst();
        break;
      case OUTPUT:
        break;
      case DECM:
        DECLARE_MEMORY_inst();
        break;
      }
    } else {
      GReg0 = (int)UnknownInstruction;
      HandleExceptions();
    }
    IncrementCSP();
  }
}
