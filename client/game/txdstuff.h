
#pragma once

int FindTxdSlot(char* szName);
int AddTxdSlot(char* szName);
bool LoadTxd(int iIndex, char* szFileName);
void AddRef(int iIndex);
void PushCurrentTxd();
void PopCurrentTxd();
void SetCurrentTxd(int iIndex);
void RemoveTxd(int iIndex);
void RemoveTxdSlot(int iIndex);
unsigned int GetNumRefs(int iIndex);
