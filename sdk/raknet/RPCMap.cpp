/// \file
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "RPCMap.h"
#include "RakAssert.h"

#include <stdlib.h>

RPCMap::RPCMap()
{
	rpcNode = NULL;
}

RPCMap::~RPCMap()
{
	Clear();
}

void RPCMap::Clear(void)
{
	struct RPCNode* n;

	while (rpcNode)
	{
		n = rpcNode->rpcNext;
		free(rpcNode);
		rpcNode = n;
	}
}

RPCNode* RPCMap::GetNodeFromID(UniqueID uniqueIdentifier)
{
	struct RPCNode* n;
	
	n = rpcNode;
	while (n)
	{
		if (n->uniqueIdentifier == uniqueIdentifier)
		{
			return n;
		}
		n = n->rpcNext;
	}
	return NULL;
}

// Called from the user thread for the local system
void RPCMap::AddIdentifierWithFunction(UniqueID uniqueIdentifier, void* functionPointer, bool isPointerToMember)
{
	RakAssert(functionPointer);

	struct RPCNode* n1, * n2;

	if (rpcNode == NULL)
	{
		rpcNode = (RPCNode*)malloc(sizeof(RPCNode));
		if (rpcNode)
		{
			rpcNode->uniqueIdentifier = uniqueIdentifier;
			rpcNode->functionPointer = functionPointer;
			rpcNode->isPointerToMember = isPointerToMember;
			rpcNode->rpcNext = NULL;
		}
	}
	else
	{
		n1 = rpcNode;
		while (n1->rpcNext != NULL)
		{
			if (n1->uniqueIdentifier == uniqueIdentifier)
			{
				n1->uniqueIdentifier = uniqueIdentifier;
				n1->functionPointer = functionPointer;
				n1->isPointerToMember = isPointerToMember;
				return;
			}
			n1 = n1->rpcNext;
		}

		n2 = (RPCNode*)malloc(sizeof(RPCNode));
		if (n2)
		{
			n2->uniqueIdentifier = uniqueIdentifier;
			n2->functionPointer = functionPointer;
			n2->isPointerToMember = isPointerToMember;
			n2->rpcNext = NULL;
			n1->rpcNext = n2;
		}
	}
}

void RPCMap::RemoveNode(UniqueID uniqueIdentifier)
{
	struct RPCNode* n1, * n2;

	n1 = rpcNode;
	if (n1->uniqueIdentifier == uniqueIdentifier)
	{
		rpcNode = n1->rpcNext;
		free(n1);
		return;
	}

	n2 = n1;
	while (n1)
	{
		if (n1->uniqueIdentifier == uniqueIdentifier)
		{
			n2->rpcNext = n1->rpcNext;
			free(n1);
			return;
		}
		n2 = n1;
		n1 = n1->rpcNext;
	}
}
