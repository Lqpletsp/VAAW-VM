#include "Decoder.h"
#include "../VirtualMachine/VM.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

static int AppendChar(char **str, size_t *len, size_t *capacity, char ch){
    if (*len >= *capacity - 1){
        *capacity = ((*capacity) == 0) ? 4 : (*capacity) * 2; 
        char *temp = realloc(*str, *capacity);
        if (temp == NULL) return 1; 
        *str = temp; 
    }
    (*str)[(*len)++] = ch; 
    (*str)[(*len)] = '\0'; 
    return 0; 
}
typedef struct{
    char *lt; 
    VM_Instructions vinst; 
} InstMap; 
typedef struct{
    char *lt;
    VM_Keywords vkey; 
} KeywordMap; 
typedef struct{
    char *lt; 
    VM_DataTypes vdtp; 
} DataTypeMap; 

static InstMap imap[] = {
    {"SEI", SETI},
    {"SEW", SETW}, 
    {"SEC", SETC}, 
    {"SEA", SETA}, 
    {"OUT", OUTPUT}, 
    {"GOT", GOTO}, 
}; 
static KeywordMap vkmap[] = {
    {"T", TRUE}, 
    {"F", FALSE},
}; 
static DataTypeMap dtpmap[] = {
    {"STR", VM_STRING}, 
    {"INT", VM_INTEGER}, 
    {"DOB", VM_DOUBLE},
    {"BOL", VM_BOOLEAN}, 
    {"CHR", VM_CHAR}, 
    {"ADR", VM_ADDRESS},
};
static int GetIndexOfDataType(char *token){
    for (size_t i = 0; i < sizeof(dtpmap)/sizeof(dtpmap[0]); ++i){
        if (strcmp(vkmap[i].lt, token) == 0) return i; 
    }
    return -1; 
}
static VM_DataTypes GetDataType(int idx){return dtpmap[idx].vdtp;}

static int GetIndexOfKeyword(char *token){
    for (size_t i = 0; i < sizeof(vkmap)/sizeof(vkmap[0]); ++i){
        if (strcmp(vkmap[i].lt, token) == 0) return i ;
    }
    return -1;
}
static VM_Keywords GetKeyword(int idx){return vkmap[idx].vkey;} 

static int GetIndexOfInst(char *token){
    for (size_t i = 0; i < sizeof(imap)/sizeof(imap[0]); ++i){
        if (strcmp(imap[i].lt, token) == 0) return i; 
    }
    return -1;
}
static VM_Instructions GetInstruction(int idx){return imap[idx].vinst;}

int DecodeFile(char *filename){
    if (GetByteCodeState() == NULL){
        printf("[SECURITY@DECODER: ByteCodePool is NULL]\n"); 
        return 1; 
    }
    FILE* fileptr = fopen("filename", "r"); 
    if (fileptr == NULL){
        printf("[SECURITY@DECODER: Cannot access the intended file]\n");
        return 1; 
    }
    char *token = NULL, PastToken = '\0'; 
    size_t tlen = 0, tcapacity = 0; 
    int ch; 
    bool FirstToken = true;
    VM_DataTypes dtp;
    Code code; 
    bool quotation = false;
    char PastQuotation = '\0';

    while ((ch = fgetc(fileptr)) != EOF){
        if (quotation) {
            if (ch == PastQuotation) {
                quotation = false; 
                PastQuotation = '\0';
            } else {
                AppendChar(&token, &tlen, &tcapacity, ch);
            }
        } 
        else if (ch == '\"' || ch == '\'') {
            quotation = true;
            PastQuotation = ch; 
        } 
        else if (ch == '\n' || ch == ' '){
            if (token != NULL && tlen > 0) {
                if (FirstToken){
                    int INST_idx = GetIndexOfInst(token); 
                    if (INST_idx < 0){
                        int KEY_idx = GetIndexOfKeyword(token); 
                        if (KEY_idx < 0){
                            int DTP_idx = GetIndexOfDataType(token);
                            if (DTP_idx < 0){
                                printf("[SECURITY@DECODER: Unknown token given\n]"); 
                                return 1; 
                            }
                            dtp = GetDataType(DTP_idx); 
                        } else{
                            code.main_type = VMKeywords; 
                            code.sub_type.vkey = GetKeyword(KEY_idx); 
                        }
                    }else{
                        code.main_type = VMInstructions; 
                        code.sub_type.vinst = GetInstruction(INST_idx);
                    }
                    AppendCodeStream(code);
                    FirstToken = false; 
                    free(token); 
                    tlen = 0;
                    tcapacity = 0;
                } else {
                    switch (dtp) {
                        case VM_ADDRESS: 
                            code.sub_type.vvmdtp = VM_ADDRESS; 
                            code.lt.data_ilt = atoi(token);
                            break; 
                        case VM_BOOLEAN:
                            code.sub_type.vvmdtp = VM_BOOLEAN; 
                            if (strcmp(token, "T") == 0) code.lt.data_blt = true; 
                            else if (strcmp(token, "F") == 0) code.lt.data_blt = false; 
                            else{
                                printf("[SECURITY@DECODER: Invalid boolean data given]\n");
                                free(token); 
                                return 1; 
                            }
                            break;
                        case VM_INTEGER: 
                            code.sub_type.vvmdtp = VM_INTEGER;  
                            code.lt.data_ilt = atoi(token);  
                            break; 
                        case VM_CHAR:
                            code.sub_type.vvmdtp = VM_CHAR;
                            if (tlen != 1){
                                printf("[SECURITY@DECODER]: Invalid char format given of length %d\n", (int)tlen);    
                                free(token);
                                return 1; 
                            }
                            code.lt.data_clt = token[0]; 
                            break; 
                        case VM_STRING:
                            code.sub_type.vvmdtp = VM_STRING; 
                            strcpy(code.lt.data_slt, token); 
                            break; 
                        case VM_DOUBLE: 
                            code.sub_type.vvmdtp = VM_DOUBLE; 
                            code.lt.data_dlt = atof(token);
                            break;
                    }
                    code.main_type = Constant;
                    AppendCodeStream(code); 
                    code.lt.data_slt = NULL; 
                    token = NULL; 
                    tlen = 0;
                    tcapacity = 0;
                }
            }
            if (ch == '\n') {
                FirstToken = true;
            }
        } 
        else {
            AppendChar(&token, &tlen, &tcapacity, ch);
        }
    }
    return 0; 
}