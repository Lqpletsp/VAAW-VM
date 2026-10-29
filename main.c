#include "Decoder/Decoder.h"
#include "VirtualMachine/VM.h"
#include <stdio.h>
int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("Invalid format\nExpected: <executable name> <.vaw file>");
    return 1;
  }
  if (InitializeByteCode()) {
    printf("[SECURITY@DECODER: Bytecode register uninitialized]\n");
    printf("[STOPPING NOW]\n");
    return 1;
  };
  if (DecodeFile(argv[1]) > 0) {
    printf("[STOPPING NOW]\n");
    return 1;
  }
  if (!GetByteCodeState()) {
    printf("[SECURITY@BRIDGE: Bytecode register uninitialized]\n");
    printf("[STOPPING NOW]\n");
    return 1;
  }
  ExecuteByteCode();
  return 0;
}
