#pragma once 
#include "../Bridge.h"
int InitializeByteCode(); 
int AppendCodeStream(Code code);
bool GetByteCodeState(); 
void InitializeSys(); 
void ExecuteByteCode(); 