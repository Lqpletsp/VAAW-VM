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
  GarbageArgForDECM,
  MemoryAlreadyDeclared,
  MemoryInitializationFailed,
  UnknownInstruction,
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
    "Argument given for DECM is not INTEGER",
    "Attempted to declare memory more than once",
    "Memory initialization failed",
    "Unrecognized instruction"};
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
static void EnqueueWBuffer() {
  // stores the value of GReg1 in WBuffer
  if (WBuffer.size >= WBuffer.capacity) {
    GReg0 = (int)FullWBuffer;
    HandleExceptions();
  }
  if (WBuffer.rear >= WBuffer.capacity)
    WBuffer.rear = 0;
  WBuffer.collection[WBuffer.rear] = GReg1;
  WBuffer.rear += 1;
  WBuffer.size += 1;
}
static void DequeueWBuffer() {
  if (WBuffer.size == 0) {
    GReg0 = (int)EmptyWBuffer;
    HandleExceptions();
  }
  WBuffer.collection[WBuffer.front] = 0;
  WBuffer.front += 1;
  if (WBuffer.front >= WBuffer.capacity)
    WBuffer.front = 0;
  WBuffer.size -= 1;
}
static void GetFrontWBuffer() {
  // stores the front value to GReg0
  if (WBuffer.size == 0) {
    GReg0 = (int)EmptyWBuffer;
    HandleExceptions();
  }
  GReg0 = WBuffer.collection[WBuffer.front];
}
static void GetBackWBuffer() {
  if (WBuffer.size == 0) {
    GReg0 = (int)EmptyWBuffer;
    HandleExceptions();
  }
  GReg0 = WBuffer.collection[WBuffer.rear];
}
static void AtWBuffer() {
  // uses the index in GReg1 and stores the data in that index to GReg2
  if (GReg1 > WBuffer.capacity) {
    GReg0 = (int)WBufferOverflow;
    HandleExceptions();
  }
  GReg0 = WBuffer.collection[GReg1];
}

static void EnqueueIBuffer() {
  // stores the value of GReg1 in WBuffer
  if (IBuffer.size >= IBuffer.capacity) {
    GReg0 = (int)FullWBuffer;
    HandleExceptions();
  }
  if (IBuffer.rear >= IBuffer.capacity)
    IBuffer.rear = 0;
  IBuffer.collection[IBuffer.rear] = GReg1;
  IBuffer.rear += 1;
  IBuffer.size += 1;
}
static void DequeueIBuffer() {
  if (IBuffer.size == 0) {
    GReg0 = (int)EmptyIBuffer;
    HandleExceptions();
  }
  IBuffer.collection[IBuffer.front] = 0;
  IBuffer.front += 1;
  if (IBuffer.front >= IBuffer.capacity)
    IBuffer.front = 0;
  IBuffer.size -= 1;
}

static void GetFrontIBuffer() {
  // stores the front value to GReg0
  if (IBuffer.size == 0) {
    GReg0 = (int)EmptyIBuffer;
    HandleExceptions();
  }
  GReg0 = IBuffer.collection[IBuffer.front];
}

static void GetBackIBuffer() {
  if (IBuffer.size == 0) {
    GReg0 = (int)EmptyIBuffer;
    HandleExceptions();
  }
  GReg0 = IBuffer.collection[IBuffer.rear];
}

static void AtIBuffer() {
  // uses the index in GReg1 and stores the data in that index to GReg2
  if (GReg1 > IBuffer.capacity) {
    GReg0 = (int)IBufferOverflow;
    HandleExceptions();
  }
  GReg0 = IBuffer.collection[GReg1];
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

static void AllocateGCache1() {}
static void IncrementCSP() {
  ++CSP;
  CC = &ByteCode.stream[CSP];
}
static void SET_IN_IBUFFER() {
  IncrementCSP();
  switch (CC->main_type) {
  case Constant:
    switch (CC->sub_type.vvmdtp) {
    case VM_CHAR: {
      GReg1 = (int)CC->lt.data_clt;
      EnqueueIBuffer();
      break;
    }
    case VM_STRING: {
      GReg4 = 0;
      while (CC->lt.data_slt[GReg4] != '\0') {
        GReg1 = (int)CC->lt.data_slt[GReg4];
        EnqueueIBuffer();
        ++GReg4;
      }
      break;
    }
    case VM_BOOLEAN: {
      if (CC->lt.data_blt) {
        // true
        GReg4 = 0;
        while (GReg4 < 4) {
          GReg1 = (int)"true"[GReg4];
          EnqueueIBuffer();
          ++GReg4;
        }
      } else {
        // false
        GReg4 = 0;
        while (GReg4 < 5) {
          GReg1 = (int)"false"[GReg4];
          EnqueueIBuffer();
          ++GReg4;
        }
      }
      break;
    }
    case VM_INTEGER:
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
      TopCBuffer();
      while (GReg0 != -1) {
        GReg1 = GReg0;
        EnqueueIBuffer();
        PopCBuffer();
        TopCBuffer();
      }
      PopCBuffer(); // to clear -1
      break;
    case VM_DOUBLE:
      break;
    case VM_ADDRESS:
      break;
    }
    GReg1 = -1;
    EnqueueIBuffer();
    break;
  default:
    // exception
    break;
  }
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
  GetFrontIBuffer();
  GReg4 = GReg0;
  while (GReg0 != -1) {
    write(1, &GReg4, 1);
    // arg: 1->stdout, 2-> buffer, 3->number of bytes to read
    DequeueIBuffer();
    GetFrontIBuffer();
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
    EnqueueIBuffer();
    ++GReg4;
  }
  GReg1 = -1;
  EnqueueIBuffer();
  // -1 is the null terminator for int array (queue) used by OUTPUT_inst
  OUTPUT_inst();
  exit(1);
}
void ExecuteByteCode() {
  IncrementCSP();
  while (CSP < ByteCode.size) {
    if (CC->main_type == VMInstructions) {
      switch (CC->sub_type.vinst) {
      case SETI:
      case SETW:
      case SETA:
      case SETC:
      case GOTO:
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
  }
}
