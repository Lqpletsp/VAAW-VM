#pragma once 
#include <string>
#include <unordered_map>
#include "../DTP.hpp"

namespace DF{
    struct rtrVal {
        int result; 
        std::string eval;
    };
    enum class DTFD{
        INTEGER, 
        IINTEGER, 
        IDOUBLE, 
        IBOOLEAN, 
        ICHAR,
        DOUBLE, 
        BOOLEAN, 
        CHAR, 
        STRING, 
        VID, 
        ADDRESS, 
    };
    inline std::unordered_map<std::string,Dat::Instructions> InstMap = {
        {"SETV", Dat::SETV},
        {"GOTOLN", Dat::GOTOLN}, 
        {"OUTPUT", Dat::OUTPUT}, 
        {"INPUT", Dat::INPUT}, 
        {"MLC", Dat::MLC}, 
        {"SETIB", Dat::SETIB}, 
        {"SETCB", Dat::SETCB}, 
        {"SETWB", Dat::SETWB},
        {"EndInstructionFlag", Dat::EndInstructionFlag}, 
        {"EndExecutionFlag", Dat::EndExecutionFlag}, 
        {"ChangeModeFlag", Dat::ChangeModeFlag},
    };
    inline std::unordered_map<std::string,DTFD> DataTypesMap = {
        {"INTEGER", DTFD::INTEGER}, 
        {"DOUBLE", DTFD::DOUBLE},
        {"BOOLEAN", DTFD::BOOLEAN}, 
        {"CHAR", DTFD::CHAR}, 
        {"STRING", DTFD::STRING}, 
        {"VID", DTFD::VID},
        {"IINTEGER", DTFD::IINTEGER}, 
        {"IDOUBLE", DTFD::IDOUBLE}, 
        {"IBOOLEAN", DTFD::IBOOLEAN}, 
        {"ICHAR", DTFD::ICHAR},
    };
    inline std::unordered_map<std::string, Dat::KeyWords> KeyWordMaps = {
        {"TRUE", Dat::TRUE}, 
        {"FALSE", Dat::FALSE}, 
        {"LESSTHAN", Dat::LESSTHAN}, 
        {"GREATEREQUAL", Dat::GREATEREQUAL}, 
        {"GREATERTHAN", Dat::GREATERTHAN}, 
        {"EQUAL", Dat::EQUAL}, 
        {"NOTEQUAL", Dat::NOTEQUAL},
        {"AND", Dat::AND}, 
        {"OR", Dat::OR}, 
        {"ADD", Dat::ADD}, 
        {"MINUS", Dat::MINUS}, 
        {"MULTIPLY", Dat::MULTIPLY}, 
        {"DIVIDE", Dat::DIVIDE}, 
        {"EXPONENT", Dat::EXPONENT}
    };
    class Decoder{
        public: 
            // methods
            int Decode_main(const std::string& dir_path); 
            int GenerateByteCode(); 
            int GetMemoryDeclared(); 
            void CleanUp(); 
            void DebugVarFnc();
            std::unordered_map<int, Dat::VDT> *GetIdenTable(); 
        private: 
            //attributes
            std::string DecodedIR; 
            std::string DecodedVariable; 
            std::string DecodedMemory;
            int MemoryDeclared; 
            std::unordered_map<int, Dat::VDT> IdenMap; 
            //methods
            // used by Decode_main
            rtrVal DecodeIR(const std::string &fp); 
            rtrVal DecodeVar(const std::string &fp);
            rtrVal DecodeMemory(const std::string &fp);
            // used by GenerateByteCode
            rtrVal iniIR(); 
            rtrVal iniVar(); 
            rtrVal iniMemory();
            bool ValidateTypes(std::string &Data, DTFD DTP);
            bool ValidateVID(int VarID);

    };
}
