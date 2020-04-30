#pragma once

#include <string>
#include <list>
#include <map>
#include <Windows.h>

class FileNode;
class CThreadPool;

class FileNodeList
{
//	HANDLE hSection;
	FileNode** nodearray;
public:
	long size;
	FileNodeList();
	virtual ~FileNodeList();
	FileNodeList* PutFromList(std::list<FileNode*>& nodelist);
	FileNode* Put_(FileNode* node);
	FileNodeList* CopyList();
	FileNode*& operator [](int ii);
	int Clear();
};

class FileNodeHelper
{
public:
	static std::wstring GetPath(FileNode* node);
	static FileNode* PutChild_(FileNode* pnode, FileNode* node);
	static int MarkUpdate(FileNode* node, DWORD dcat);
	static DWORD UpdateNode(FileNode* node);
	static int UpdateNodeExtIndex(FileNode* node, std::map<std::wstring, int>& mapext, int* mapcnt, int mapcntsize);
	static int UpdateNodeColor(FileNode* node, DWORD* colors, int cnt);
};

static const DWORD nuc_size = 0x1;
static const DWORD nuc_compsize = 0x2;
static const DWORD nuc_fcnt = 0x4;
static const DWORD nuc_dcnt = 0x8;
static const DWORD nuc_progress = 0x10;

static const DWORD nuc_all = nuc_size | nuc_compsize | nuc_fcnt | nuc_dcnt | nuc_progress;

class FileNode
{
public:
	enum NodeType
	{
		NT_DIR
		, NT_FILE
		, NT_VDIR
	} type;
	WCHAR* name;
	FileNode* parent;
	FileNodeList nodes;
	int filecnt;
	int dircnt;
	long long size;
	long long compsize;
	double progress;
	DWORD updated_cat;
	int version;
	int extindex;
	DWORD color;

	FileNode();
	virtual ~FileNode();
	//FileNode* PutChild(FileNode* node);
	//std::wstring GetPath();
};
