#pragma once 
#include "../Bridge.h"
int InitializeByteCode(); 
int AppendCodeStream(Code code);
Code *GetByteCodeState(); 
void InitializeSys(); 
void ExecuteByteCode(); 