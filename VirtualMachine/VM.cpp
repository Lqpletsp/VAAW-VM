#include "VM.hpp"
#include "../DTP.hpp"
#include <iostream>
#include <cstdlib>
#include <cstring>
namespace VMF{
bool VirtualMachine::iniMemmory(int x){
    try{
       TotalMemoryDeclared = x; 
       for (size_t i = TotalMemoryDeclared; i > 0; --i){
            VirtualVolatileMemory.push_back({});
            g_MemoryPool.push(i);
       }
       BCP = 0; 
    } catch(...){
        return false; 
    }
    return true; 
}

void VirtualMachine::ReallocVMemory(){g_MemoryPool.push(GBuffer1);}
void VirtualMachine::AllocateMemoryInWBuffer(){WBuffer.push_back(GBuffer1);}
void VirtualMachine::AllocateMemoryInCBuffer(){CBuffer.push(GBuffer1);}
void VirtualMachine::AllocateMemoryInIBuffer(){IBuffer.push_back(GBuffer1);}
void VirtualMachine::SetIdentifierMap(std::unordered_map<int, Dat::VDT> *Map){IdentifierMap = Map;}

void VirtualMachine::ClearTopCBuffer(){
    if (CBuffer.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::EmptyCBuffer);
        HandleException();
    }
    GBuffer1 = CBuffer.top();
    ReallocVMemory();
    CBuffer.pop(); 
}

void VirtualMachine::ClearTopIBuffer(){
    if (IBuffer.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::EmptyIBuffer);
        HandleException();
    }
    GBuffer1 = IBuffer.front();
    ReallocVMemory();
    IBuffer.pop_front(); 
}

void VirtualMachine::ClearTopWBuffer(){
    if (WBuffer.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::EmptyWBuffer);
        HandleException();
    }
    GBuffer1 = WBuffer.front();
    ReallocVMemory();
    WBuffer.pop_front(); 
}

void VirtualMachine::GetTopWBufferAddress(){
    if (WBuffer.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::EmptyWBuffer);
        HandleException();
    }
    GBuffer1 = WBuffer.front(); 
}

void VirtualMachine::GetTopCBufferAddress(){
    if (CBuffer.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::EmptyCBuffer);
        HandleException();
    }
    GBuffer1 = CBuffer.top(); 
}

void VirtualMachine::GetTopIBufferAddress(){
    if (IBuffer.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::EmptyIBuffer);
        HandleException();
    }
    GBuffer1 = IBuffer.front(); 
}

void VirtualMachine::GetXIBufferAddress(){
    if (GBuffer2 >= IBuffer.size()) {
        GBuffer0 = static_cast<int>(VMExceptions::OutOfBoundsInIBuffer);
        HandleException();
    }
    GBuffer1 = IBuffer.at(GBuffer2); 
}

void VirtualMachine::GetXWBufferAddress(){
    if (GBuffer2 >= WBuffer.size()) {
        GBuffer0 = static_cast<int>(VMExceptions::OutOfBoundsInWBuffer);
        HandleException();
    }
    GBuffer1 = WBuffer.at(GBuffer2); 
}

void VirtualMachine::AllocateMemory(){
    if (g_MemoryPool.empty()) {
        GBuffer0 = static_cast<int>(VMExceptions::RanOutOfVirtualVolatileMemory);
        HandleException();
    }
    GBuffer0 = g_MemoryPool.top();
    g_MemoryPool.pop(); 
}
void VirtualMachine::ClearCBuffer(){
    while (!CBuffer.empty()){
        GBuffer1 = CBuffer.top(); 
        ReallocVMemory();
        CBuffer.pop();  
    }
}
void VirtualMachine::ClearIBuffer(){
    while (!IBuffer.empty()){
        GBuffer1 = CBuffer.top(); 
        ReallocVMemory(); 
        IBuffer.pop_back();
    }
}
void VirtualMachine::ClearWBuffer(){
    while (!WBuffer.empty()){
        GBuffer1 = CBuffer.top(); 
        ReallocVMemory(); 
        WBuffer.pop_back();
    }
}

void VirtualMachine::LoadInCBuffer(){
    if (DPBuffer.DTP == Dat::DataTypes::STRING){
        GBuffer3 = 0; 
        while (GBuffer3 < GBuffer2){
            AllocateMemory(); 
            GBuffer1 = GBuffer0;  
            AllocateMemoryInCBuffer(); 
            VirtualVolatileMemory.at(GBuffer1).Data = DPBuffer.Data[GBuffer3];
            VirtualVolatileMemory.at(GBuffer1).DT = Dat::CHAR;  
            ++GBuffer3; 
        }
    } else{
        AllocateMemory(); 
        GBuffer1 = GBuffer0;  
        AllocateMemoryInCBuffer(); 
        VirtualVolatileMemory.at(GBuffer1).Data = DPBuffer.Data[0]; 
        VirtualVolatileMemory.at(GBuffer1).DT = DPBuffer.DTP; 
    }
    AllocateMemory(); 
    GBuffer1 = GBuffer0; 
    AllocateMemoryInCBuffer(); 
    VirtualVolatileMemory.at(GBuffer1).Data = "\0"; 
    VirtualVolatileMemory.at(GBuffer1).DT = Dat::CHAR; 
}
void VirtualMachine::LoadInWBuffer(){
    if (DPBuffer.DTP == Dat::DataTypes::STRING){
        GBuffer3 = 0; 
        while (GBuffer3 < GBuffer2){
            AllocateMemory(); 
            GBuffer1 = GBuffer0;  
            AllocateMemoryInWBuffer(); 
            VirtualVolatileMemory.at(GBuffer1).Data = DPBuffer.Data[GBuffer3];
            VirtualVolatileMemory.at(GBuffer1).DT = Dat::CHAR;  
            ++GBuffer3; 
        }
    } else{
        AllocateMemory(); 
        GBuffer1 = GBuffer0;  
        AllocateMemoryInWBuffer(); 
        VirtualVolatileMemory.at(GBuffer1).Data = DPBuffer.Data[0]; 
        VirtualVolatileMemory.at(GBuffer1).DT = DPBuffer.DTP; 
    }
    AllocateMemory(); 
    GBuffer1 = GBuffer0; 
    AllocateMemoryInWBuffer(); 
    VirtualVolatileMemory.at(GBuffer1).Data = "\0"; 
    VirtualVolatileMemory.at(GBuffer1).DT = Dat::CHAR; 
}
void VirtualMachine::StoreInAddr(){
    VirtualVolatileMemory.at(GBuffer1).Data = VirtualVolatileMemory.at(GBuffer0).Data;   
    VirtualVolatileMemory.at(GBuffer1).DT = VirtualVolatileMemory.at(GBuffer0).DT; 
}
void VirtualMachine::LoadInIBuffer(){
    if (DPBuffer.DTP == Dat::DataTypes::STRING){
        GBuffer3 = 0; 
        while (GBuffer3 < GBuffer2){
            AllocateMemory(); 
            GBuffer1 = GBuffer0;  
            AllocateMemoryInIBuffer(); 
            VirtualVolatileMemory.at(GBuffer1).Data = DPBuffer.Data[GBuffer3];
            VirtualVolatileMemory.at(GBuffer1).DT = Dat::CHAR;  
            ++GBuffer3; 
        }
    } else{
        AllocateMemory(); 
        GBuffer1 = GBuffer0;  
        AllocateMemoryInIBuffer(); 
        VirtualVolatileMemory.at(GBuffer1).Data = DPBuffer.Data[0]; 
        VirtualVolatileMemory.at(GBuffer1).DT = DPBuffer.DTP; 
    }
    AllocateMemory(); 
    GBuffer1 = GBuffer0; 
    AllocateMemoryInIBuffer(); 
    VirtualVolatileMemory.at(GBuffer1).Data = "\0"; 
    VirtualVolatileMemory.at(GBuffer1).DT = Dat::CHAR; 
}
void VirtualMachine::LoadBCInCBuffer(){
    switch(Dat::ByteCode.at(BCP).TType){
        case Dat::Constant:
            (Dat::ByteCode.at(BCP).LT.size() <= 255) ? 
            GBuffer2 = Dat::ByteCode.at(BCP).LT.size() : GBuffer2 = 255; 
            std::strcpy(DPBuffer.Data.data(), Dat::ByteCode.at(BCP).LT.c_str());
            DPBuffer.DTP = Dat::ByteCode.at(BCP).Sub_TType.DT; 
            LoadInCBuffer();
            break;
        case Dat::Identifier: 
            VBuffer = &IdentifierMap->at(std::stoi(Dat::ByteCode.at(BCP).LT)); 
            if (VBuffer->MAA.empty()){
                GBuffer0 = static_cast<int>(VMExceptions::AttemptedAccessToUninitializedVariable); 
                HandleException(); 
            }
            GBuffer2 = 0;
            GBuffer3 = VBuffer->MAA.size();  
            while (GBuffer2 < GBuffer3){
                std::strcpy(DPBuffer.Data.data(), Dat::ByteCode.at(BCP).LT.c_str());
                DPBuffer.DTP = Dat::ByteCode.at(BCP).Sub_TType.DT; 
                LoadInCBuffer();
                ++GBuffer2; 
            }
        default: break;  
    }
}
void VirtualMachine::LoadBCInIBuffer(){
    switch(Dat::ByteCode.at(BCP).TType){
        case Dat::Constant:
            (Dat::ByteCode.at(BCP).LT.size() <= 255) ? 
            GBuffer2 = Dat::ByteCode.at(BCP).LT.size() : GBuffer2 = 255; 
            std::strcpy(DPBuffer.Data.data(), Dat::ByteCode.at(BCP).LT.c_str());
            DPBuffer.DTP = Dat::ByteCode.at(BCP).Sub_TType.DT; 
            LoadInIBuffer();
            break;
        case Dat::Identifier: 
            VBuffer = &IdentifierMap->at(std::stoi(Dat::ByteCode.at(BCP).LT)); 
            if (VBuffer->MAA.empty()){
                GBuffer0 = static_cast<int>(VMExceptions::AttemptedAccessToUninitializedVariable); 
                HandleException(); 
            }
            GBuffer2 = 0;
            GBuffer3 = VBuffer->MAA.size();  
            while (GBuffer2 < GBuffer3){
                std::strcpy(DPBuffer.Data.data(), Dat::ByteCode.at(BCP).LT.c_str());
                DPBuffer.DTP = Dat::ByteCode.at(BCP).Sub_TType.DT; 
                LoadInIBuffer();
                ++GBuffer2; 
            }
            break; 
        default: break;  
    }
}
void VirtualMachine::LoadBCInWBuffer(){
    switch(Dat::ByteCode.at(BCP).TType){
        case Dat::Constant:
        (Dat::ByteCode.at(BCP).LT.size() <=  255) ? 
            GBuffer2 = Dat::ByteCode.at(BCP).LT.size() : GBuffer2 = 255; 
            std::strcpy(DPBuffer.Data.data(), Dat::ByteCode.at(BCP).LT.c_str());
            DPBuffer.DTP = Dat::ByteCode.at(BCP).Sub_TType.DT; 
            LoadInWBuffer();
            break;
        case Dat::Identifier: 
            VBuffer = &IdentifierMap->at(std::stoi(Dat::ByteCode.at(BCP).LT)); 
            if (VBuffer->MAA.empty()){
                GBuffer0 = static_cast<int>(VMExceptions::AttemptedAccessToUninitializedVariable); 
                HandleException(); 
            }
            GBuffer2 = 0;
            GBuffer3 = VBuffer->MAA.size();  
            while (GBuffer2 < GBuffer3){
                std::strcpy(DPBuffer.Data.data(), Dat::ByteCode.at(BCP).LT.c_str());
                DPBuffer.DTP = Dat::ByteCode.at(BCP).Sub_TType.DT; 
                LoadInWBuffer();
                ++GBuffer2; 
            }
            break; 
        default: break;  
    }
}
void VirtualMachine::BufferCleanUps(){
    ClearCBuffer(); 
    ClearIBuffer(); 
    ClearWBuffer(); 
}
void VirtualMachine::HandleException(){
    BufferCleanUps();
    GBuffer1 = 0; 
    GBuffer2 = VirtualMachine::ExceptionMessage.at(MapExceptionCodeAndMessage.at(GBuffer0)).size(); 
    std::snprintf(
        DPBuffer.Data.data(), 
        DPBuffer.Data.size(), 
        "[EXCP@BCP%d] : %s", 
        BCP, 
        VirtualMachine::ExceptionMessage.at(MapExceptionCodeAndMessage.at(GBuffer0)).c_str()
    );
    DPBuffer.DTP = Dat::STRING;
    LoadInIBuffer();
    GBuffer1 = std::string("[EXCP@BCP" + std::to_string(BCP) + "] : ").size(); 
    GBuffer2 = 0; 
    GBuffer3 = VirtualMachine::ExceptionMessage.at(MapExceptionCodeAndMessage.at(GBuffer0)).size() + GBuffer1;
    while (GBuffer2 != GBuffer3){
        OUTPUT(); 
        ++GBuffer2; 
    }
    std::exit(1);
}

void VirtualMachine::OUTPUT(){
    GetTopIBufferAddress(); 
    while(VirtualVolatileMemory.at(GBuffer1).Data != "\0"){
        std::cout << VirtualVolatileMemory.at(GBuffer1).Data; 
        ClearTopIBuffer(); 
        if (!IBuffer.empty()) GetTopIBufferAddress(); 
        else break; 
    };  
}
void VirtualMachine::SET_IN_WBUFFER(){
    ++BCP;
    LoadBCInWBuffer(); 
}
void VirtualMachine::SET_IN_IBUFFER(){
    ++BCP; 
    LoadBCInIBuffer(); 
}
void VirtualMachine::SET_IN_CBUFFER(){
    ++BCP; 
    LoadBCInCBuffer(); 
}
void VirtualMachine::LoadInVID(){
    // expects identifier ID in GBuffer1 
    DebugVarFnc(); 
    VBuffer = &IdentifierMap->at(GBuffer1); 
    GetTopWBufferAddress(); 
    GBuffer2 = 0;
    // GBuffer1 gets overridden by GetTopWBufferAddress() so can use that as an index
    while(VirtualVolatileMemory.at(GBuffer1).Data != "\0"){
        if (GBuffer2 >= VBuffer->MAA.size()){
            AllocateMemory(); 
            VBuffer->MAA.push_back(GBuffer0); 
            VirtualVolatileMemory.at(GBuffer0).DT = VBuffer->DT; 
        }
        GBuffer4 = VBuffer->MAA.at(GBuffer2); 
        VirtualVolatileMemory.at(VBuffer->MAA.at(GBuffer2)) = VirtualVolatileMemory.at(GBuffer1);  
        ClearTopWBuffer(); 
        GetTopWBufferAddress(); 
        ++GBuffer2; 
    };  
    std::cout << "EXITLOADINVID\n";
}
void VirtualMachine::SET_IN_IDENTIFIER(){
    ++BCP; 
    if (Dat::ByteCode.at(BCP).TType != Dat::Identifier &&
     Dat::ByteCode.at(BCP).TType != Dat::ADDRESS){
        GBuffer0 = static_cast<int>(VMExceptions::ExpectingVIDOrAddress); 
        HandleException(); 
    }
    if (Dat::ByteCode.at(BCP).TType == Dat::ADDRESS){
        GetTopWBufferAddress(); 
        GBuffer0 = GBuffer1; 
        GBuffer1 = std::stoi(Dat::ByteCode.at(BCP).LT); 
        StoreInAddr(); 
    } else {
        GBuffer1 = std::stoi(Dat::ByteCode.at(BCP).LT);
        LoadInVID();
        // VID loading complete
    } 
}
void VirtualMachine::INPUT(){}
void VirtualMachine::CHANGE_BCP(){
    ++BCP; 
    try{
        BCP = std::stoi(Dat::ByteCode.at(BCP).LT) - 1; 
        // decrement the count because the instruction loop will just increment BCP when it is below the while loop
        // so instead of BCP, the value will be BCP + 1  
    } catch(...){
        GBuffer0 = static_cast<int>(VMExceptions::GarbageValueForBCP); 
        HandleException(); 
    }
}
void VirtualMachine::MLC(){}
int VirtualMachine::ExecuteByteCode(){
    while (BCP < Dat::ByteCode.size() && BCP >= 0){
        switch(Dat::ByteCode.at(BCP).TType){
            case (Dat::VMInstruction):{
                switch(Dat::ByteCode.at(BCP).Sub_TType.VMInstruction){
                    case Dat::OUTPUT: 
                        OUTPUT(); 
                        break; 
                    case Dat::SETV: 
                        SET_IN_IDENTIFIER(); 
                        break;
                    case Dat::SETIB:
                        SET_IN_IBUFFER();
                        break; 
                    case Dat::SETCB: 
                        SET_IN_CBUFFER(); 
                        break; 
                    case Dat::SETWB: 
                        SET_IN_WBUFFER(); 
                        break;
                    case Dat::MLC: 
                    case Dat::EndExecutionFlag: 
                    case Dat::EndInstructionFlag: 
                    case Dat::GOTOLN: 
                    case Dat::INPUT: 
                    case Dat::ChangeModeFlag:
                        break;
                }
            }
            default: break; 
        }
        ++BCP; 
    }
    return 0;
} 
void VirtualMachine::DebugVarFnc(){
    for (const auto it : *IdentifierMap){
        std::cout << it.first << ' ' << static_cast<int>(it.second.DT) << '\n';
    }
}
}