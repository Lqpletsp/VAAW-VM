#pragma once 
#include <string>
#include <vector>
#include <deque>

namespace Dat{
    enum BCT { // BCT -> Bytecode Types
    Constant,
    Identifier,
    VMInstruction,
    VMKeywords,
    };

    enum DataTypes {
    INTEGER,
    DOUBLE,
    BOOLEAN,
    CHAR,
    STRING,
    };

    enum KeyWords {
    TRUE,
    FALSE,
    Arr,
    ArrEND,
    LESSTHAN,
    GREATERTHAN,
    LESSEQUAL,
    GREATEREQUAL,
    EQUAL,
    NOTEQUAL,
    AND,
    OR,
    ADD,
    MINUS,
    MULTIPLY,
    DIVIDE,
    EXPONENT,
    };

    enum Instructions {
    SETV,
    GOTOLN,
    OUTPUT,
    INPUT,
    MLC,
    SETWB,
    SETCB, 
    SETIB,  
    EndInstructionFlag,
    EndExecutionFlag,
    ChangeModeFlag,
    };

    struct ByteCodeType{
        std::string LT; 
        BCT TType;  
        union{
            Instructions VMInstruction; 
            KeyWords KeyWord;
            DataTypes DT; 
        } Sub_TType;
    };

    inline std::vector<ByteCodeType> ByteCode;

    struct RawMemoryAddress{
        DataTypes DT; 
        std::string Data;
    };

    struct VDT{
        int VID; // Variable ID 
        DataTypes DT; // Data Type
        std::deque<int> MAA; // Memory Adresses Assigned
        bool Immutable = false;  
    };

};
