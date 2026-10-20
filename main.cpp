#include "VirtualMachine/VM.hpp"
#include "Decoder/Decoder.hpp"
#include <iostream>

int main(int argc, char *argv[]){
    // execution format: ./amcVM <Asmmic directory> 
    if (argc != 2) {
        std::cerr << "Invalid execution format\nExpected: ./VAAWVM <Asmmic directory>\n";
        return 1;
    }
    DF::Decoder Decoder_main;
    int x = Decoder_main.Decode_main(argv[1]);
    if (x > 0){ 
        std::cerr << "[STOPPING NOW]\n";
        return 1; 
    }
    x = Decoder_main.GenerateByteCode(); 
    if (x > 0){ 
        std::cerr << "[STOPPING NOW]\n";
        return 1; 
    }
    Decoder_main.CleanUp();
    Decoder_main.DebugVarFnc(); 
    std::cout << "[BRIDGE: Bytecode generation complete]\n";
    VMF::VirtualMachine VirtualMachine_main;
    VirtualMachine_main.iniMemmory(Decoder_main.GetMemoryDeclared());
    VirtualMachine_main.SetIdentifierMap(Decoder_main.GetIdenTable());
    x = VirtualMachine_main.ExecuteByteCode();
    if (x > 0) return 1;
    return 0; 
}