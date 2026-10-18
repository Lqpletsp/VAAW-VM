#pragma once
#include <deque>
#include <stack>
#include <array>
#include <unordered_map>
#include "../DTP.hpp"
namespace VMF{

    class VirtualMachine{
        public:
            //methods
            bool iniMemmory(int x);
            int ExecuteByteCode();
            int SetMemory(int MemoryDeclared); 
            void SetIdentifierMap(std::unordered_map<int, Dat::VDT> *Map); 
        private: 
            // attributes
            int TotalMemoryDeclared; 
            int BCP; 
            std::vector<Dat::RawMemoryAddress> VirtualVolatileMemory; 
            // buffer data structures store virtual memory addresses
            // buffer variables can store any int value
            std::deque<int> IBuffer; // Input/Ouput Buffer
            std::deque<int> WBuffer; // Working Buffer
            std::stack<int> CBuffer; // Computational Buffer 
            int GBuffer0 = 0; // General Buffer 0 
            int GBuffer1 = 0; // General Buffer 1 
            int GBuffer2 = 0; // General Buffer 2
            int GBuffer3 = 0; // General Buffer 3 
            Dat::VDT *VBuffer; // Variable Buffer
            struct DPair{
                std::array<char, 256> Data; 
                Dat::DataTypes DTP; 
            }; 
            DPair DPBuffer; // Data Pair Buffer
            std::unordered_map<int, Dat::VDT> *IdentifierMap; 
            std::stack<int> g_MemoryPool;
            //methods
            void OUTPUT();
            void SET_IN_WBUFFER(); 
            void SET_IN_CBUFFER(); 
            void SET_IN_IBUFFER(); 
            void SET_IN_IDENTIFIER(); 
            void CHANGE_BCP(); 
            void INPUT(); 
            void MLC(); 
            std::vector<Dat::RawMemoryAddress>* GetVariableMAA(int VID);
            void GetByteCodeValue();
            void StoreInAddr(); 
            void LoadInWBuffer(); // for all types of values
            void LoadInIBuffer(); // for all types of values
            void LoadInCBuffer(); // for all types of values
            void LoadBCInWBuffer(); // stores the data of the bytecode in WBuffer
            void LoadBCInIBuffer(); // stores the data of the bytecode in IBuffer
            void LoadBCInCBuffer(); // stores the data of the bytecode in CBuffer
            void AllocateMemoryInCBuffer(); 
            void AllocateMemoryInWBuffer(); 
            void AllocateMemoryInIBuffer(); 
            void ReallocVMemory();
            void ClearIBuffer(); 
            void ClearWBuffer(); 
            void ClearCBuffer();
            void BufferCleanUps(); 
            void ClearTopWBuffer(); 
            void ClearTopCBuffer(); 
            void ClearTopIBuffer(); 
            void ClearTopGBuffer(); 
            void AllocateMemory(); 
            void GetTopWBufferAddress();
            void GetTopIBufferAddress(); 
            void GetTopCBufferAddress(); 
            void GetXWBufferAddress(); 
            void GetXIBufferAddress(); 
            // exceptions
            enum class VMExceptions{
                RanOutOfVirtualVolatileMemory, 
                EmptyWBuffer, 
                EmptyIBuffer, 
                EmptyCBuffer,  
                StringDataInMemory, 
                StringDataInIBuffer, 
                StringDataInWBuffer, 
                StringDataInCBuffer, 
                OutOfBoundsInIBuffer, 
                OutOfBoundsInWBuffer, 
                UnIdentifiedInstruction, 
                AttemptedAccessToUninitializedVariable, 
                GarbageValueForBCP,
            };
            const std::unordered_map<int, VMExceptions> MapExceptionCodeAndMessage = {
                {0,  VMExceptions::RanOutOfVirtualVolatileMemory},
                {1,  VMExceptions::EmptyWBuffer},
                {2,  VMExceptions::EmptyIBuffer},
                {3,  VMExceptions::EmptyCBuffer},
                {4,  VMExceptions::StringDataInMemory},
                {5,  VMExceptions::StringDataInIBuffer},
                {6,  VMExceptions::StringDataInWBuffer},
                {7,  VMExceptions::StringDataInCBuffer},
                {8,  VMExceptions::OutOfBoundsInIBuffer},
                {9,  VMExceptions::OutOfBoundsInWBuffer},
                {10, VMExceptions::UnIdentifiedInstruction},
                {11, VMExceptions::AttemptedAccessToUninitializedVariable},
                {12, VMExceptions::GarbageValueForBCP}, 
            };
            const std::unordered_map<VMExceptions,std::string> ExceptionMessage{
                {VMExceptions::EmptyCBuffer, "Computation buffer empty"},
                {VMExceptions::EmptyIBuffer, "Input/Output buffer empty"}, 
                {VMExceptions::EmptyWBuffer, "Working buffer empty"}, 
                {VMExceptions::RanOutOfVirtualVolatileMemory, "All virtual volatile memory used"},
                {VMExceptions::StringDataInMemory, "String data cannot be stored in memory as a whole"},
                {VMExceptions::OutOfBoundsInIBuffer, "Index given is out of bounds for IBuffer"},
                {VMExceptions::OutOfBoundsInWBuffer, "Index given is out of bounds for WBuffer"}, 
                {VMExceptions::UnIdentifiedInstruction, "Invalid bytecode instruction"}, 
                {VMExceptions::StringDataInCBuffer, "String data cannot be stored in CBuffer as a whole"}, 
                {VMExceptions::StringDataInIBuffer, "String data cannot be stored in IBuffer as a whole"}, 
                {VMExceptions::StringDataInWBuffer, "String data cannot be stored in WBuffer as a whole"},
                {VMExceptions::AttemptedAccessToUninitializedVariable, "Identifier attempted to access is uninitialized"},
                {VMExceptions::GarbageValueForBCP, "BCP must be of type INTEGER but was not given"},
            };
            void HandleException(); 
    };
}
