#include "Decoder.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include "../DTP.hpp"


namespace fs = std::filesystem;
namespace DF{
// getters 
std::unordered_map<int, Dat::VDT> *DF::Decoder::GetIdenTable(){return &IdenMap;}
int DF::Decoder::GetMemoryDeclared(){return MemoryDeclared;}

rtrVal Decoder::DecodeMemory(const std::string &fp){
    std::ifstream file(fp);
    if (!file.is_open()){
        return {1, "[Cannot open file]\n"}; 
    }
    DecodedMemory = std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    file.close();
    return {0, ""}; 
}
rtrVal Decoder::DecodeIR(const std::string& fp) {
    std::ifstream file(fp); 
    if (!file.is_open()) {
        return {1, "[Cannot open file]\n"}; 
    }

    DecodedIR = std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    file.close();
    return {0, ""}; 
}
rtrVal Decoder::DecodeVar(const std::string& fp) {
    std::ifstream file(fp); 
    if (!file.is_open()) {
        return {1, "[Cannot open file\n]"}; 
    }

    DecodedVariable = std::string(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );
    file.close();
    return {0, ""}; 
}


int Decoder::Decode_main(const std::string& dir_path) {
    if (!fs::exists(dir_path) || !fs::is_directory(dir_path)) {
        std::cout << "]Error opening directory \"" << dir_path << "\"]\n"; 
        return 1; 
    }
    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (!entry.is_regular_file()) continue;

        std::string filename = entry.path().filename().string();
        std::string ext = entry.path().extension().string();

        if (ext.empty()) {
            std::cerr << "[SECURITY: Directory does not conform to the rules of the VM decoder]\n"
                      << "\"" << filename << "\" has no extension\n"; 
            return 1; 
        }

        if (ext != ".dsm") {
            std::cerr << "[SECURITY: Directory does not conform to the rules of the VM decoder]\n"
                      << "\"" << filename << "\" has an invalid extension\n"; 
            return 1; 
        } 

        if (filename != "memory.dsm" && filename != "variables.dsm" && filename != "ir.dsm") {
            std::cerr << "[SECURITY: Directory does not conform to the rules of the VM decoder]\n"; 
            return 1; 
        }
    }
    fs::path path = fs::path(dir_path) / "memory.dsm";
    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        std::cerr << "[SECURITY: Memory file not decoded]\n";
        return 1;
    }
    std::cerr << "[File recognized: memory.dsm]\n";
    rtrVal rtr = Decoder::DecodeMemory(path.string());
    if (rtr.result > 0) {
        std::cerr << rtr.eval << '\n'; 
        return 1;
    }
    std::cout << "[Memory decoding complete]\n";

    path = fs::path(dir_path) / "variables.dsm";
    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        std::cerr << "SECURITY: Variable file not decoded\n";
        return 1;
    }
    std::cerr << "[File recognized: variable.dsm]\n";
    rtr = Decoder::DecodeVar(path.string());
    if (rtr.result > 0) {
        std::cerr << rtr.eval << '\n'; 
        return 1;
    }
    std::cout << "[Variable decoding complete]\n";

    path = fs::path(dir_path) / "ir.dsm";
    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        std::cerr << "[SECURITY: IR file not decoded]\n";
        return 1;
    }
    std::cerr << "[File recognized: ir.dsm]\n";
    rtr = Decoder::DecodeIR(path.string());
    if (rtr.result > 0) {
        std::cerr << rtr.eval << '\n'; 
        return 1;
    }
    std::cout << "[IR decoding complete]\n";

    return 0; 
}
bool Decoder::ValidateTypes(std::string &Data, DTFD DTP){
    try{
        std::stod(Data);
        if (DTP == DTFD::BOOLEAN) return false; 
        if (DTP == DTFD::CHAR && Data.size() > 1) return false;  
    } catch(...){
        if (DTP == DTFD::INTEGER || DTP == DTFD::DOUBLE || DTP == DTFD::VID)  return false; 
        if (DTP == DTFD::BOOLEAN  && (Data != "T" && Data != "F")) return false; 
        if (DTP == DTFD::CHAR && Data.size() != 1) return false; 
    }
    return true; 
}
bool Decoder::ValidateVID(int VarID){
    auto it = IdenMap.find(VarID); 
    if (it == IdenMap.end()) return false; 
    return true;
}
rtrVal Decoder::iniVar(){
    bool IDMode = true;
    bool Immutable = false; 
    Dat::VDT Variable; 
    int ID = -1;   
    std::string token; 
    for (const char ch : DecodedVariable){
        if (ch == ' ' || ch == '\n'){
            if (IDMode){
                if (!ValidateTypes(token, DTFD::INTEGER))  
                    return {1,
                 "[SECURITY: Non integer value was given for variable ID during identifier initialization]"};
                ID = std::stoi(token); 
                IdenMap[ID] = {}; 
            } else {
                if (ID == -1) return {1, "[SECURITY: Invalid variable file syntax]"}; 
                auto it =DataTypesMap.find(token); 
                if (it == DataTypesMap.end()) 
                    return {1, "[SECURITY: Unrecognized type \"" + token + "\" given]"}; 
                switch(it->second){
                    case DTFD::BOOLEAN:
                        Variable.DT = Dat::BOOLEAN; 
                        break; 
                    case DTFD::DOUBLE: 
                        Variable.DT = Dat::DOUBLE; 
                        break;
                    case DTFD::CHAR:
                        Variable.DT = Dat::CHAR; 
                        break; 
                    case DTFD::INTEGER:
                        Variable.DT = Dat::INTEGER; 
                        break; 
                    case DTFD::IINTEGER: 
                        Variable.DT = Dat::INTEGER; 
                        Immutable = true; 
                        break; 
                    case DTFD::IBOOLEAN: 
                        Variable.DT = Dat::BOOLEAN; 
                        Immutable = true; 
                        break; 
                    case DTFD::ICHAR: 
                        Variable.DT = Dat::CHAR;
                        Immutable = true; 
                        break; 
                    case DTFD::IDOUBLE: 
                        Variable.DT = Dat::DOUBLE; 
                        Immutable = true; 
                        break;
                    default: 
                        return {1, "[SECURITY: Unrecognized type \"" + token + "\" given]"}; 
                }
                if (Immutable) Variable.Immutable = true; 
                else Variable.Immutable = false; 
                Variable.VID = ID; 
                IdenMap[ID] = Variable;
                ID = -1;
            } 
        } else { 
           token.push_back(ch); 
        }
    }
    return {0};
}
rtrVal Decoder::iniIR(){
    if (DecodedIR.empty()) return {0};
    bool CommandMode = true, VID = false, quotation = false; 
    std::string token; 
    char PastQuotation = '\0'; 
    Dat::ByteCodeType BT; 
    DTFD DT; 
    Dat::DataTypes BCType; 

    for (const char ch : DecodedIR){
        if (quotation) {
            if (ch == PastQuotation) {
                quotation = false; 
                PastQuotation = '\0';
            } else {
                token.push_back(ch);
            }
        } 
        else if (ch == '\"' || ch == '\'') {
            quotation = true;
            PastQuotation = ch; 
        } 
        else if (ch == ' ' || ch == '\n') {
            if (!token.empty()) {
                if (CommandMode){
                    auto it = InstMap.find(token); 
                    if (it == InstMap.end()){
                        auto dtIt = DataTypesMap.find(token); 
                        if (dtIt == DataTypesMap.end()) {
                            return {1, "[SECURITY: Command \"" + token + "\" not found]"};
                        }
                        switch (dtIt->second) {
                            case DTFD::BOOLEAN: BCType = Dat::BOOLEAN; break; 
                            case DTFD::INTEGER: BCType = Dat::INTEGER; break; 
                            case DTFD::CHAR: BCType = Dat::CHAR; break; 
                            case DTFD::STRING: BCType = Dat::STRING; break; 
                            case DTFD::VID: VID = true; break;
                            case DTFD::DOUBLE: BCType = Dat::DOUBLE; break;
                            default: 
                                return {1, "[SECURITY: Unidentified data type \"" + token + "\" used]"};
                        }
                        token.clear();
                        DT = dtIt->second;
                        CommandMode = false;
                        continue; 
                    }
                    BT.TType = Dat::VMInstruction; 
                    BT.Sub_TType.VMInstruction = it->second;
                    token.clear();
                    Dat::ByteCode.push_back(BT);
                    CommandMode = false;
                } else {
                    if (!ValidateTypes(token, DT)){ 
                        return {.result = 1, .eval = "[SECURITY: Invalid data types for token]"};
                    }
                    BT.LT = token; 
                    if (VID){
                        try{
                            ValidateVID(std::stoi(token));
                            BT.TType = Dat::Identifier; 
                        } catch(...){
                            return {.result = 0, .eval = "[SECURITY: Expected INTEGER for variable ID but was not given]"};
                        }
                        VID = false; 
                    } else {
                        BT.TType = Dat::Constant;
                        BT.Sub_TType.DT = BCType;
                    }
                    token.clear();
                    Dat::ByteCode.push_back(BT);
                }
            }
            if (ch == '\n') CommandMode = true;
        } 
        else {
            token.push_back(ch);
        }
    }
    return {0};
}
rtrVal Decoder::iniMemory(){
    try{
        MemoryDeclared = std::stoi(DecodedMemory); 
    } catch(...){
        return {1, "[SECURITY: Memory value should be of type INTEGER, but was not given]"};
    }
    return {0};
}
int Decoder::GenerateByteCode(){ 
    rtrVal rtr = iniMemory(); 
    if (rtr.result > 0){
        std::cerr << rtr.eval << '\n';
        return 1; 
    }
    rtr = iniVar(); 
    if (rtr.result > 0) {
        std::cerr << rtr.eval << '\n'; 
        return 1 ; 
    }
    rtr = iniIR(); 
    if (rtr.result > 0){
        std::cerr << rtr.eval << '\n'; 
        return 1; 
    }
    return 0;
}

void Decoder::CleanUp(){
    std::string().swap(DecodedIR); 
    std::string().swap(DecodedMemory); 
    std::string().swap(DecodedVariable); 
}
void Decoder::DebugVarFnc(){
    for (const auto it : IdenMap){
        std::cout << it.first << ' ' << static_cast<int>(it.second.DT) << '\n'; 
    }
}
}