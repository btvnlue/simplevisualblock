#include "VisualBlock.h"

#include <list>

VisualBlock::~VisualBlock()
{
	for (std::list<VisualBlock*>::iterator itbk = blocks.begin()
		; itbk != blocks.end()
		; itbk++)
	{
		delete* itbk;
	}
	blocks.clear();
	nodes.clear();
}
