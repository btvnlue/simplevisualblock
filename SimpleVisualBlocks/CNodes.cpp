#include "CNodes.h"

#include <sstream>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")

#define NODE_COLOR_UNINDEX -1
#define NO_COLOR_INDEX -2

FileNodeList::FileNodeList()
	:size(0)
	, nodearray(NULL)
{
}

FileNodeList::~FileNodeList()
{
	if (nodearray)
	{
		delete nodearray;
		nodearray = NULL;
		size = 0;
	}
}

FileNodeList* FileNodeList::PutFromList(std::list<FileNode*>& nodelist)
{
	FileNode** nna = new FileNode * [nodelist.size()];
	int ii = 0;
	for (std::list<FileNode*>::iterator itfn = nodelist.begin()
		; itfn != nodelist.end()
		; itfn++) {
		nna[ii] = *itfn;
		ii++;
	}
	nodearray = nna;
	size = nodelist.size();
	return this;
}

FileNode* FileNodeList::Put_(FileNode* node)
{
	size++;
	FileNode** nna = new FileNode * [size];
	if (nodearray) {
		memcpy_s(nna, sizeof(FileNode*) * size, nodearray, sizeof(FileNode*) * (size - 1));
		delete nodearray;
	}
	nna[size - 1] = node;
	nodearray = nna;
	return node;
}

FileNodeList* FileNodeList::CopyList()
{
	FileNodeList* nlst = NULL;
	nlst = new FileNodeList();
	if (size > 0)
	{
		nlst->nodearray = new FileNode* [size];
		memcpy_s(nlst->nodearray, sizeof(FileNode*) * size, nodearray, sizeof(FileNode*) * size);
		nlst->size = size;
	}

	return nlst;
}

FileNode*& FileNodeList::operator [](int ii)
{
	return nodearray[ii];
}

int FileNodeList::Clear()
{
	if (size > 0)
	{
		for (int ii = 0; ii < size; ii++)
		{
			delete nodearray[ii];
		}
		delete nodearray;
		nodearray = NULL;
		size = 0;
	}
	return 0;
}

FileNode::FileNode()
	:name(NULL)
	, parent(NULL)
	, type(NT_DIR)
	, filecnt(0)
	, dircnt(0)
	, size(0)
	, progress(0)
	, updated_cat(0)
	, version(0)
	, compsize(0)
	, extindex(NODE_COLOR_UNINDEX)
	, color(RGB(0xFF,0xFF,0xFF))
{
}

FileNode::~FileNode()
{
	nodes.Clear();
	if (name)
	{
		delete name;
	}
}

std::wstring FileNodeHelper::GetPath(FileNode* node)
{
	std::wstringstream wss;
	if (node->type == FileNode::NT_VDIR) {
		if (node->parent)
		{
			wss << FileNodeHelper::GetPath(node->parent);
		}
		else
		{
			wss << L"";
		}
	} else {
		if (node->parent)
		{
			wss << FileNodeHelper::GetPath(node->parent) << L"\\" << node->name;
		}
		else
		{
			wss << node->name;
		}
	}  

	return wss.str();
}

//FileNode* FileNodeHelper::PutChild_(FileNode* pnode, FileNode* node)
//{
//	node->parent = pnode;
//	pnode->nodes.Put(node);
//
//	return node;
//}

int FileNodeHelper::MarkUpdate(FileNode* node, DWORD dcat)
{
	if (node)
	{
		if (node->parent)
		{
			MarkUpdate(node->parent, dcat);
		}
		node->updated_cat |= dcat;
	}
	return 0;
}

DWORD FileNodeHelper::UpdateNode(FileNode* node)
{
	DWORD cat = node->updated_cat;
	FileNode* cnd;
	if (cat)
	{
		node->updated_cat = 0;
		if (node->type != FileNode::NT_FILE) {
			if (cat & nuc_dcnt) {
				node->dircnt = 0;
			}
			if (cat & nuc_fcnt) {
				node->filecnt = 0;
			}
			if (cat & nuc_size) {
				node->size = 0;
			}
			if (cat & nuc_compsize) {
				node->compsize = 0;
			}
			if (cat & nuc_progress) {
				node->progress = 0;
			}

			FileNodeList* fnl = node->nodes.CopyList();
			if (fnl->size > 0)
			{
				for (int ii = 0; ii < fnl->size; ii++)
				{
					cnd = (*fnl)[ii];
					if (cnd->type == FileNode::NT_DIR || cnd->type == FileNode::NT_VDIR)
					{
						UpdateNode(cnd);
					}
					if (cat & nuc_dcnt) {
						if (cnd->type == FileNode::NT_DIR) {
							node->dircnt++;
						}
					}
					if (cat & nuc_fcnt) {
						node->filecnt += cnd->filecnt;
					}
					if (cat & nuc_dcnt) {
						node->dircnt += cnd->dircnt;
					}
					if (cat & nuc_size) {
						node->size += cnd->size;
					}
					if (cat & nuc_compsize) {
						node->compsize += cnd->compsize;
					}
					if (cat & nuc_progress) {
						node->progress += cnd->progress * cnd->filecnt;
					}
				}
				if (cat & nuc_progress) {
					if (node->filecnt > 0) {
						node->progress /= node->filecnt;
					}
					else {
						node->progress = 1;
					}
				}
			}
			else
			{
				if (cat & nuc_progress) {
					node->progress = 1;
				}
			}
			delete fnl;
		}
	}

	return cat;
}

int FileNodeHelper::UpdateNodeExtIndex(FileNode* node, std::map<std::wstring, int>& mapext, int* mapcnt, int mapcntsize)
{
	wchar_t npath[MAX_PATH] = { 0 };
	if (node->type == FileNode::NT_FILE) {
		if (node->extindex == NODE_COLOR_UNINDEX) {
			wcscpy_s(npath, node->name);
			wchar_t* ext = PathFindExtension(npath);
			int extlen = wcslen(ext);
			if (extlen > 0) {
				CharLower(ext);
				if (mapext.find(ext) == mapext.end()) {
					mapext[ext] = mapext.size();
				}
				node->extindex = mapext[ext];

			}
			else {
				node->extindex = NO_COLOR_INDEX;
			}
		}
		if (node->extindex >= 0) {
			if (node->extindex < mapcntsize) {
				mapcnt[node->extindex]++;
			}
		}
	}
	else {
		FileNodeList* nnl = node->nodes.CopyList();
		for (int ii = 0; ii < nnl->size; ii++) {
			UpdateNodeExtIndex((*nnl)[ii], mapext, mapcnt, mapcntsize);
		}
		delete nnl;
	}
	return 0;
}

int FileNodeHelper::UpdateNodeColor(FileNode* node, DWORD* colors, int cnt)
{
	if (node->type == FileNode::NT_FILE) {
		if (node->extindex >= 0) {
			if (node->extindex < cnt) {
				node->color = colors[node->extindex];
			}
		}
	}
	else {
		FileNodeList* nnl = node->nodes.CopyList();
		for (int ii = 0; ii < nnl->size; ii++) {
			UpdateNodeColor((*nnl)[ii], colors, cnt);
		}
		delete nnl;
	}
	return 0;
}
