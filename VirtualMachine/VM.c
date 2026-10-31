#include "VM.h"
#include "../Bridge.h"
#include <complex.h>
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
static int FReg0;    // Flag Register
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
  int *collection;
  int size;
  int front;
  int rear;
  int capacity;
} MPool;
static special_queue GCache0;
static MPool MemoryPool;
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
  EmptyGCache,
  GarbageArgForDECM,
  MemoryAlreadyDeclared,
  MemoryInitializationFailed,
  UnknownInstruction,
  NonIntConstantForCSP,
  GarbageArgForGOTO,
  GarbageArgForSETA,
  AttemptedOutOfBoundsCSP,
  UnknownFRegCode,
  RanOutOfMemory,
  InvalidMemoryAddressAssignment
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
    "CSP changed is out-of-bounds for the current bytecode",
    "Unknown FReg0 code",
    "Memory full",
    "Attempted invalid memory address assignment in memory pool"};
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
static void DequeueGCache0() {
  if (GCache0.size == 0) {
    GReg0 = -1;
    return;
  }
  GCache0.collection[GCache0.front] = 0;
  GCache0.front += 1;
  if (GCache0.front >= GCache0.capacity)
    GCache0.front = 0;
  GCache0.size -= 1;
}
static void EnqueueMPool() {
  // altering the MemoryPool may lead to bugs regarding addresses
  if (MemoryPool.size >= MemoryPool.capacity) {
    GReg0 = (int)InvalidMemoryAddressAssignment;
    FReg0 = 2;
    return;
  }
  if (MemoryPool.rear >= MemoryPool.capacity)
    MemoryPool.rear = 0;
  MemoryPool.rear += 1;
  MemoryPool.size += 1;
}
static void DequeueMPool() {
  if (MemoryPool.size == 0) {
    GReg0 = (int)RanOutOfMemory;
    FReg0 = 2;
    return;
  }
  MemoryPool.front += 1;
  if (MemoryPool.front >= MemoryPool.capacity)
    MemoryPool.front = 0;
  MemoryPool.size -= 1;
}
static void TopMPool() {
  if (MemoryPool.size == 0) {
    GReg0 = (int)RanOutOfMemory;
    FReg0 = 2;
    return;
  }
  GReg1 = MemoryPool.collection[MemoryPool.front];
}
static void EnqueueGCache0() {
  // stores the value of GReg1 in GCache0
  if (GCache0.size >= GCache0.capacity) {
    DequeueGCache0();
    return;
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
    return;
  }
  GReg0 = GCache0.collection[GCache0.front];
}

static void GetBackGCache0() {
  if (GCache0.size == 0) {
    GReg0 = -1;
    return;
  }
  GReg0 = GCache0.collection[GCache0.rear];
}

static void AtGCache0() {
  // uses the index in GReg1 and stores the data in that index to GReg2
  if (GReg1 > GCache0.capacity) {
    GReg0 = -1;
    return;
  }
  GReg0 = GCache0.collection[GReg1];
}
static void PushCBuffer() {
  // Pushes the value of GReg1
  if (CBuffer.size >= CBuffer.capacity) {
    GReg0 = (int)FullCBuffer;
    FReg0 = 2;
    return;
  }
  CBuffer.collection[CBuffer.top] = GReg1;
  CBuffer.top += 1;
  CBuffer.size += 1;
}
static void TopCBuffer() {
  if (CBuffer.size == 0) {
    GReg0 = (int)EmptyCBuffer;
    FReg0 = 2;
    return;
  }
  GReg0 = CBuffer.collection[CBuffer.top];
}
static void PopCBuffer() {
  if (CBuffer.size == 0) {
    GReg0 = (int)EmptyCBuffer;
    FReg0 = 2;
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
  // start address at GReg0
  GReg4 = 0;
  while (VirtualVolatileMemory[GReg0 + GReg4].data.data_clt != '\0') {
    // arg: 1->stdout, 2-> buffer, 3->number of bytes to read
    write(1, &VirtualVolatileMemory[GReg0 + GReg4].data.data_ilt, 1);
    ++GReg4;
  }
}

static void HandleExceptions() {
  // searches for error code in GReg0
  GReg4 = 0;
  MemoryPool.front = 0;
  MemoryPool.rear = 0;
  MemoryPool.size = 0;
  TopMPool();
  while (ExceptionMessages[GReg0][GReg4] != '\0') {
    VirtualVolatileMemory[GReg1 + GReg4].data.data_clt =
        ExceptionMessages[GReg0][GReg4];
    ++GReg4;
  }
  VirtualVolatileMemory[GReg1 + GReg4].data.data_clt = '\0';
  GReg0 = GReg1;
  OUTPUT_inst();
}
static void SETA_inst() {
  // format: SETA <destination> <source>
  IncrementCSP();
  if (CC->main_type != Constant && CC->sub_type.vvmdtp != VM_ADDRESS) {
    GReg0 = (int)GarbageArgForSETA;
    FReg0 = 2;
    return;
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
        FReg0 = 2;
        return;
      }
      GReg0 = CC->lt.data_ilt;
      if (GReg0 >= ByteCode.size || GReg0 < 0) {
        GReg0 = (int)AttemptedOutOfBoundsCSP;
        FReg0 = 2;
        return;
      }
      break;
    case VM_ADDRESS:
      GReg0 = CC->lt.data_ilt;
      if (GReg0 >= ByteCode.size || GReg0 < 0) {
        GReg0 = (int)AttemptedOutOfBoundsCSP;
        FReg0 = 2;
        return;
      }
      break;
    default:
      GReg0 = (int)GarbageArgForGOTO;
      FReg0 = 2;
      return;
      break;
    }
    CSP = GReg0;
    CC = &ByteCode.stream[CSP];
  }
  default:
    GReg0 = (int)GarbageArgForGOTO;
    FReg0 = 2;
    return;
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
        OUTPUT_inst();
        break;
      case DECM:
        DECLARE_MEMORY_inst();
        break;
      case END:
        FReg0 = 1;
        break;
      }
      if (FReg0 != 0)
        return;
    } else {
      GReg0 = (int)UnknownInstruction;
      FReg0 = 2;
      return;
    }
    IncrementCSP();
  }
}
void mini_os() {
  while (true) {
    ExecuteByteCode();
    switch (FReg0) {
    case 1:
      return;
    case 2:
      HandleExceptions();
      break;
    default:
      GReg0 = (int)UnknownFRegCode;
      HandleExceptions();
    }
    FReg0 = 0;
  }
}
bool Boot(int memory) {
  if (memory < 0)
    return false;
  GCache0.capacity = 256;
  GCache0.front = 0;
  GCache0.rear = 0;
  GCache0.size = 0;
  CBuffer.capacity = 256;
  CBuffer.top = 0;
  CBuffer.size = 0;
  TotalMemory = memory;
  MemoryPool.rear = 0;
  MemoryPool.front = 0;
  MemoryPool.size = 0;
  VirtualVolatileMemory =
      (RawMemorySpace *)malloc(TotalMemory * sizeof(RawMemorySpace));
  MemoryPool.collection = (int *)malloc(memory * sizeof(int));
  if (VirtualVolatileMemory == NULL || MemoryPool.collection == NULL)
    return false;
  MemoryPool.capacity = memory;
  GReg0 = memory - 1;
  while (GReg0 >= 0) {
    MemoryPool.collection[GReg0] = GReg0;
    --GReg0;
  }
  mini_os();
  return true;
}
