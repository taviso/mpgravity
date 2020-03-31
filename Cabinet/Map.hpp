///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Author: Elm� (http://netcult.ch/elmue)
// Date: 23-11-2006
//
// Filename: Map.hpp
//
// Classes:
// - CMap
// - kStore
//
// Purpose: This tiny class is a simple (and ultra fast) replacement for std::map
// It is strongly recommended to use this class intead, because using #include <map>
// will create a dependency to MSVCP70.DLL or MSVCP71.DLL or MSVCP80.DLL
// If you use std::map you have to deliver one of these DLLs with you application 
// because these DLLs are not available on every computer.
// Using CMap instead removes this problem!
//

#pragma once

#include "String.hpp"

namespace Cabinet
{

// Dynamic memory management is not necessary for this simple class
// we will never need more than 2 entries! (the handles of the CAB file)
// If the maximum is exceeded an exception will be thrown, 
// but this will never happen as long as CMap is used only in the cabinet library
#define __MaxEntries 10

template<class cKeyType, class cDataType>
class CMap
{
public:
	CMap()
	{
		Clear();
	}

	void Clear()
	{
		ms32_Count = 0;
	}

	int Count()
	{
		return ms32_Count;
	}

	int Find(cKeyType i_Key)
	{
		for (int i=0; i<ms32_Count; i++)
		{
			if (mi_Keys[i] == i_Key)
				return i;
		}
		return -1;
	}

	// Returns the data of the Key or null if not found
	cDataType* GetData(cKeyType i_Key)
	{
		int Pos = Find(i_Key);
		if (Pos < 0)
			return 0;

		return &mi_Data[Pos];
	}

	// Delete the given Key if it exists
	void Delete(cKeyType i_Key)
	{
		int Pos = Find(i_Key);
		if (Pos < 0)
			return;

		ms32_Count --;
		// overwrite the element which is deleted with the last element
		mi_Data[Pos] = mi_Data[ms32_Count];
		mi_Keys[Pos] = mi_Keys[ms32_Count];
	}

	// Sets the data for the given key
	// If the key already exists, its data is overwritten
	// If the key does not yet exist, it is created
	void Set(cKeyType i_Key, cDataType i_Data)
	{
		int Pos = Find(i_Key);
		if (Pos < 0)
		{
			if (ms32_Count >= __MaxEntries)
				throw "Overflow in CMap::Set()";

			Pos = ms32_Count; // create new entry
			ms32_Count ++;
		}

		mi_Data[Pos] = i_Data;
		mi_Keys[Pos] = i_Key;
	}

private:
	int        ms32_Count;
	cKeyType   mi_Keys[__MaxEntries];
	cDataType  mi_Data[__MaxEntries];
};

// Stores handles of all CAB files and their read pointer position
class CFilePtr
{
public:

	void Clear() 
	{ 
		mi_Map.Clear(); 
	}

	BOOL IsCabFile(INT_PTR s32_Handle)
	{
		return mi_Map.Find(s32_Handle) >= 0;
	}

	void Delete(INT_PTR s32_Handle)
	{
		mi_Map.Delete(s32_Handle);
	}
	
	void SetPtr(INT_PTR s32_Handle, int s32_Pointer)
	{
		if (s32_Handle == 0 || s32_Handle == -1) 
			return;
		
		mi_Map.Set(s32_Handle, s32_Pointer);
	}

	int GetPtr(INT_PTR s32_Handle)
	{
		int* ps32_Ptr = mi_Map.GetData(s32_Handle);
		if  (ps32_Ptr)
			return *ps32_Ptr;
		else 
			return -1;
	}

private:

	CMap<INT_PTR, int> mi_Map;
};

} // Namespace Cabinet


