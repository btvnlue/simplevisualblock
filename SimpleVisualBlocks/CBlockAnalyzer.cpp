#include "CBlockAnalyzer.h"
#include "VisualBlock.h"
#include <set>

CBlockAnalyzer::CBlockAnalyzer()
	:procnode(NULL)
	, SmallSizeLimit(6)
{
}

BOOL CBlockAnalyzer::CanSplit(VisualBlock* block)
{
	BOOL rtn = FALSE;
	if (block->rect.bottom - block->rect.top > SmallSizeLimit) {
		if (block->rect.right - block->rect.left > SmallSizeLimit) {
			rtn = TRUE;
		}
	}
	return rtn;
}

//class fnless : public std::less<FileNode*> {
class fnless {
public:
	constexpr bool operator()(FileNode * const & _Left, FileNode * const & _Right) const {
		if (_Left->compsize < _Right->compsize) return true;
		if (_Left->compsize > _Right->compsize) return false;
		if (_Left < _Right) return true;
		return false;
	}
};

int CBlockAnalyzer::SplitRect(RECT& rct, double ratio, RECT& lrct, RECT& rrct)
{
	long esz;
	if ((rct.bottom - rct.top) > (rct.right - rct.left)) {
		esz = rct.bottom - rct.top;
		esz = long(ratio * esz);
		if (esz < SmallSizeLimit) {
			esz = SmallSizeLimit / 2;
		}
		if ((rct.bottom - rct.top - esz) < SmallSizeLimit) {
			esz = rct.bottom - rct.top - SmallSizeLimit / 2;
		}

		lrct.left = rct.left;
		lrct.right = rct.right;
		lrct.top = rct.top;
		lrct.bottom = rct.top + esz;

		rrct.left = rct.left;
		rrct.right = rct.right;
		rrct.top = lrct.bottom + 1;
		rrct.bottom = rct.bottom;
	}
	else {
		esz = rct.right - rct.left;
		esz = long(ratio * esz);
		if (esz < SmallSizeLimit) {
			esz = SmallSizeLimit / 2;
		}
		if ((rct.right - rct.left - esz) < SmallSizeLimit) {
			esz = rct.right - rct.left - SmallSizeLimit / 2;
		}

		lrct.left = rct.left;
		lrct.right = rct.left + esz;
		lrct.top = rct.top;
		lrct.bottom = rct.bottom;

		rrct.left = lrct.right + 1;
		rrct.right = rct.right;
		rrct.top = rct.top;
		rrct.bottom = rct.bottom;
	}

	return 0;
}

int CBlockAnalyzer::SplitNodesRect(VisualBlock* ori, VisualBlock* left, VisualBlock* right)
{
	int rtn = 0;
	std::set<FileNode *, fnless> fns;
	
	long long tsize = 0;
	for (std::list<FileNode*>::const_iterator itfn = ori->nodes.begin(); itfn!=ori->nodes.end(); itfn++) {
		if ((*itfn)->compsize > 0) {
			fns.insert(*itfn);
			tsize += (*itfn)->compsize;
		}
	}
	if (fns.size() >= 1) {
		long long sts = 0;
		std::set<FileNode*> lns;
		BOOL keepseek = fns.size()>1;
		FileNode* wkn;
		while (keepseek) {
			wkn = *fns.begin();
			fns.erase(fns.begin());
			sts += wkn->compsize;
			lns.insert(wkn);
			if (sts >= tsize / 2) {
				keepseek = FALSE;
			}
			if (fns.size() <= 1) {
				keepseek = FALSE;
			}
		}
		left->nodes.insert(left->nodes.begin(), lns.begin(), lns.end());
		right->nodes.insert(right->nodes.begin(), fns.begin(), fns.end());
		//SplitRect(ori->rect, ((double)sts) / tsize, left->rect, right->rect);
		SplitRect(ori->rect, ((double)(tsize - sts)) / tsize, right->rect, left->rect);
	}
	return rtn;
}

int CBlockAnalyzer::AnalyzeBlock(VisualBlock* blk)
{
	VisualBlock* sbb;
	std::list<FileNode*> lnl, rnl;
	if (CanSplit(blk)) {
		if (blk->nodes.size() == 1) {
			FileNode* lnd = *blk->nodes.begin();
			FileNodeList* sns = lnd->nodes.CopyList();
			if (sns->size > 0) {
				sbb = new VisualBlock();
				for (int ii = 0; ii < sns->size; ii++) {
					sbb->nodes.push_back((*sns)[ii]);
				}
				sbb->rect = blk->rect;
				blk->blocks.push_back(sbb);
				AnalyzeBlock(sbb);
			}
			delete sns;
		}
		else if (blk->nodes.size() > 1) {
			VisualBlock* lblk = new VisualBlock();
			VisualBlock* rblk = new VisualBlock();
			SplitNodesRect(blk, lblk, rblk);
			blk->blocks.push_back(lblk);
			AnalyzeBlock(lblk);

			if (rblk->nodes.size() > 0) {
				blk->blocks.push_back(rblk);
				AnalyzeBlock(rblk);
			}
			else {
				delete rblk;
			}
		}
	}
	return 0;
}

VisualBlock* CBlockAnalyzer::AnalyzeFileNode(FileNode* node, RECT rct)
{
	VisualBlock* block = new VisualBlock();
	block->nodes.push_back(node);
	block->rect = rct;
	AnalyzeBlock(block);
	procnode = node;
	return block;
}

int CBlockAnalyzer::GetEdgeList(VisualBlock* block, std::list<VisualBlock*>& edglist)
{
	if (block->blocks.size() > 0) {
		for (std::list<VisualBlock*>::iterator itvb = block->blocks.begin()
			; itvb != block->blocks.end()
			; itvb++) {
			GetEdgeList(*itvb, edglist);
		}
	}
	else {
		edglist.push_back(block);
	}
	return 0;
}

int CBlockAnalyzer::GetNodeList(VisualBlock* block, std::list<VisualBlock*>& edglist)
{
	if (block->blocks.size() > 0) {
		if (block->nodes.size() == 1) {
			edglist.push_back(block);
		}

		for (std::list<VisualBlock*>::iterator itvb = block->blocks.begin()
			; itvb != block->blocks.end()
			; itvb++) {
			GetNodeList(*itvb, edglist);
		}
	}
	else {
		edglist.push_back(block);
	}
	return 0;
}

BOOL CBlockAnalyzer::NeedRedraw(int nodeupd, FileNode* nsel, BOOL resized)
{
	BOOL rtn = FALSE;

	if (nodeupd > 0) {
		rtn = TRUE;
	}
	else if (nsel != procnode) {
		rtn = TRUE;
	}
	else if (resized) {
		rtn = TRUE;
	}

	return rtn;
}

