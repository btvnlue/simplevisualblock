#include "CPathAnalyzer.h"
#include "CThreadPool.h"

#include <Windows.h>
#include <sstream>

int Log(const wchar_t* msg);


WCHAR cfiles[] = L"[Files]";

class CFileLoadTask : public CThTask
{
	FileNode* node;
public:
	CFileLoadTask(FileNode* fnd)
		: node(fnd)
	{};
	virtual int Process__() {
		std::wstring ffp = FileNodeHelper::GetPath(node);
		DWORD dhigh;
		WCHAR cbuf[1024];
		wsprintf(cbuf, L"Task: Get compress size [%s]", ffp.c_str());
		Log(cbuf);
		DWORD dlow = ::GetCompressedFileSize(ffp.c_str(), &dhigh);
		if (dlow != INVALID_FILE_SIZE) {
			LARGE_INTEGER lit;
			lit.HighPart = dhigh;
			lit.LowPart = dlow;
			node->compsize = lit.QuadPart;
		}
		node->progress = 1;
		FileNodeHelper::MarkUpdate(node, nuc_compsize | nuc_progress);

		return 0;
	}
	virtual int Process_() {
		std::wstring ffp = FileNodeHelper::GetPath(node);
		HANDLE hpf = ::CreateFile(ffp.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hpf != INVALID_HANDLE_VALUE) {
			LARGE_INTEGER lit;
			BOOL rtn = ::GetFileSizeEx(hpf, &lit);
			if (rtn) {
				node->compsize = lit.QuadPart;
			}
			CloseHandle(hpf);
		}
		node->progress = 1;
		FileNodeHelper::MarkUpdate(node, nuc_compsize | nuc_progress);

		return 0;
	}
	virtual int Process() {
		std::wstring ffp = FileNodeHelper::GetPath(node);
		HANDLE hpf = ::CreateFile(ffp.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hpf != INVALID_HANDLE_VALUE) {
			FILE_STANDARD_INFO fai = { 0 };
			BOOL btn = GetFileInformationByHandleEx(hpf, FileStandardInfo, &fai, sizeof(FILE_STANDARD_INFO));

			if (btn) {
				node->compsize = fai.AllocationSize.QuadPart;
			}
			CloseHandle(hpf);
		}
		node->progress = 1;
		FileNodeHelper::MarkUpdate(node, nuc_compsize | nuc_progress);

		return 0;
	}

};

class CVirutalDirTask : public CThTask
{
	FileNode* node;
	CThreadPool* pool;
public:
	CVirutalDirTask(CThreadPool* tpl, FileNode* fnd)
		: node(fnd)
		, pool(tpl)
	{}
	virtual int Process()
	{
		FileNodeList* nnl = node->nodes.CopyList();
		CThTask* task;
		int ii = 0;
		FileNode* wnd;
		BOOL keepseek = ii < nnl->size;

		for (int ii = 0; ii<nnl->size; ii++) {
			wnd = (*nnl)[ii];
			if (wnd->type == FileNode::NT_FILE) {
				task = new CFileLoadTask(wnd);
				pool->PutTask(task);
			}
		}
		delete nnl;
		return 0;
	}
	virtual int _Process()
	{
		WCHAR cbuf[1024];
		wsprintf(cbuf, L"Task: Virtual Node [%s]", node->name);
		Log(cbuf);
		std::wstring ffp;
		FileNode* wnd;

		FileNodeList* nnl = node->nodes.CopyList();
		CThTask* task;
		int ii = 0;
		BOOL keepseek = ii < nnl->size;
		while (keepseek) {
			task = NULL;
			wnd = (*nnl)[ii];
			if (wnd->type == FileNode::NT_FILE) {
				ffp = FileNodeHelper::GetPath(wnd);
				HANDLE hpf = ::CreateFile(ffp.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

				if (hpf != INVALID_HANDLE_VALUE) {
					FILE_STANDARD_INFO fai = { 0 };
					BOOL btn = GetFileInformationByHandleEx(hpf, FileStandardInfo, &fai, sizeof(FILE_STANDARD_INFO));

					if (btn) {
						wnd->compsize = fai.AllocationSize.QuadPart;
					}
					CloseHandle(hpf);
				}
				wnd->progress = 1;
			}
			else if (wnd->type == FileNode::NT_VDIR) {
				task = new CVirutalDirTask(pool, wnd);
			}

			if (task) {
				pool->PutTask(task);
			}

			ii++;
			keepseek = ii < nnl->size;
			keepseek = pool->keeprun ? keepseek : FALSE;

			if (ii % 100 == 0) {
				FileNodeHelper::MarkUpdate(node, nuc_compsize | nuc_progress);
			}
		}
		FileNodeHelper::MarkUpdate(node, nuc_compsize | nuc_progress);

		delete nnl;

		return 0;
	}
};

class CDirLoadTask : public CThTask
{
	CThreadPool* pool;
	FileNode* node;
public:
	CDirLoadTask(CThreadPool* tpl, FileNode* fnd)
		: pool(tpl)
		, node(fnd)
	{

	}
	virtual int Process()
	{
		WIN32_FIND_DATA wfd = { 0 };
		FileNode* nnd;
		BOOL keepseek = TRUE;
		int szz;
		FileNode* vfn = NULL;
		std::list<FileNode*> dlst;
		std::list<FileNode*> vlst;
		int dircnt = 0;

		std::wstringstream seekpath;
		seekpath << FileNodeHelper::GetPath(node);
		seekpath << "\\*";
#ifdef _DEBUG
		Log(seekpath.str().c_str());
#endif // _DEBUG

		HANDLE hff = ::FindFirstFile(seekpath.str().c_str(), &wfd);
		if (hff != INVALID_HANDLE_VALUE)
		{
			while (keepseek)
			{
				nnd = NULL;
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (wcscmp(wfd.cFileName, L".") == 0 || wcscmp(wfd.cFileName, L"..") == 0)
					{
						nnd = NULL;
					}
					else
					{
						nnd = new FileNode();
						szz = (int)wcslen(wfd.cFileName) + 1;
						nnd->name = new WCHAR[szz];
						wcscpy_s(nnd->name, szz, wfd.cFileName);
						nnd->type = FileNode::NT_DIR;
						nnd->parent = node;

						dlst.push_back(nnd);
						dircnt++;
					}
				}
				else
				{
					if (vfn == NULL)
					{
						vfn = new FileNode();
						szz = sizeof(cfiles) / sizeof(WCHAR) + 1;
						vfn->name = new WCHAR[szz];
						wcscpy_s(vfn->name, szz - 1, cfiles);
						vfn->type = FileNode::NT_VDIR;
						vfn->updated_cat = nuc_fcnt | nuc_progress | nuc_size;
						vfn->parent = node;
						dlst.push_back(vfn);
					}
					nnd = new FileNode();
					szz = (int)wcslen(wfd.cFileName) + 1;
					nnd->name = new WCHAR[szz];
					wcscpy_s(nnd->name, szz, wfd.cFileName);
					nnd->type = FileNode::NT_FILE;
					nnd->size += ULARGE_INTEGER{ wfd.nFileSizeLow, wfd.nFileSizeHigh }.QuadPart;
					nnd->filecnt = 1;
					nnd->progress = 0;
					nnd->parent = vfn;

					vlst.push_back(nnd);

					vfn->filecnt++;
					vfn->size += nnd->size;
				}
				keepseek = ::FindNextFile(hff, &wfd);
				keepseek = pool->keeprun ? keepseek : FALSE;
			}
			::FindClose(hff);

			if (vfn) {
				vfn->nodes.PutFromList(vlst);
			}
			if (dlst.size() > 0) {
				node->nodes.PutFromList(dlst);
				node->dircnt += dircnt;
			}

			CThTask* task;
			FileNodeList* nnl = node->nodes.CopyList();
			for (int ii = 0; ii < nnl->size; ii++)
			{
				task = NULL;
				if ((*nnl)[ii]->type == FileNode::NT_DIR)
				{
					task = new CDirLoadTask(pool, (*nnl)[ii]);
				}
				else if ((*nnl)[ii]->type == FileNode::NT_FILE){
					task = new CFileLoadTask((*nnl)[ii]);
				}
				else if ((*nnl)[ii]->type == FileNode::NT_VDIR) {
					task = new CVirutalDirTask(pool, (*nnl)[ii]);
				}

				if (task) {
					pool->PutTask(task);
				}
			}
			delete nnl;

		}
		else
		{
			node->progress = 1;
		}
		FileNodeHelper::MarkUpdate(node, nuc_dcnt | nuc_size | nuc_progress | (vfn == NULL?0:nuc_fcnt));

		return 0;
	}
};

//int CPathAnalyzer::StartPath(const std::wstring& path)
//{
//	root = new FileNode();
//	int szz = (int)path.size() + 1;
//	root->name_ = new WCHAR[szz];
//	wcscpy_s(root->name_, szz, path.c_str());
//	root->type = FileNode::NT_DIR;
//	LoadDirectory(root);
//	return 0;
//}

int CPathAnalyzer::StartPathDist(const std::wstring& path)
{
	root = new FileNode();
	int szz = (int)path.size() + 1;
	root->name = new WCHAR[szz];
	wcscpy_s(root->name, szz, path.c_str());
	root->type = FileNode::NT_DIR;

	if (pool)
	{
		delete pool;
	}
	pool = new CThreadPool();
	pool->CreatePool(6);

	CDirLoadTask* task = new CDirLoadTask(pool, root);
	pool->PutTask(task);

	return 0;
}


//int CPathAnalyzer::LoadDirectory(FileNode* nodedir)
//{
//	WIN32_FIND_DATA wfd = { 0 };
//	FileNode* nnd;
//	BOOL keepseek = TRUE;
//	int szz;
//
//	std::wstringstream seekpath;
//	seekpath << FileNodeHelper::GetPath(nodedir);
//	seekpath << "\\*";
//#ifdef _DEBUG
//	Log(seekpath.str().c_str());
//#endif // _DEBUG
//
//	HANDLE hff = ::FindFirstFile(seekpath.str().c_str(), &wfd);
//	if (hff != INVALID_HANDLE_VALUE)
//	{
//		while (keepseek)
//		{
//			nnd = NULL;
//			if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
//			{
//				if (wcscmp(wfd.cFileName, L".") == 0 || wcscmp(wfd.cFileName, L"..") == 0)
//				{
//					nnd = NULL;
//				}
//				else
//				{
//					nnd = new FileNode();
//					szz = (int)wcslen(wfd.cFileName) + 1;
//					nnd->name_ = new WCHAR[szz];
//					wcscpy_s(nnd->name_, szz, wfd.cFileName);
//					nnd->type = FileNode::NT_DIR;
//				}
//			}
//			else
//			{
//				nnd = new FileNode();
//				szz = (int)wcslen(wfd.cFileName) + 1;
//				nnd->name_ = new WCHAR[szz];
//				wcscpy_s(nnd->name_, szz, wfd.cFileName);
//				nnd->type = FileNode::NT_FILE;
//			}
//			if (nnd)
//			{
//				FileNodeHelper::PutChild(nodedir, nnd);
//			}
//			keepseek = ::FindNextFile(hff, &wfd);
//		}
//		::FindClose(hff);
//
//		FileNodeList* nnl = nodedir->nodes.CopyList_();
//		for (int ii=0; ii<nnl->size; ii++)
//		{
//			if ((*nnl)[ii]->type == FileNode::NT_DIR)
//			{
//				LoadDirectory((*nnl)[ii]);
//			}
//		}
//		delete nnl;
//	}
//	return 0;
//}

int CPathAnalyzer::Stop()
{
	if (pool)
	{
		pool->Stop();
		delete pool;
		pool = NULL;
	}
	if (root) {
		delete root;
		root = NULL;
	}
	return 0;
}

int CPathAnalyzer::GetTasksCount()
{
	return pool->GetTasksCount();
}



//std::wstring FileNode::GetPath()
//{
//	std::wstringstream wss;
//	if (parent)
//	{
//		wss << parent->GetPath() << L"\\" << name_;
//	}
//	else
//	{
//		wss << name_;
//	}
//	return wss.str();
//}

