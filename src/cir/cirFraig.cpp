/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
	HashSet<GateNode> gateHash(getHashSize(_DFSList.size()));
	CirGate* tmpPtr = NULL;
	bool r = 0;
	for(size_t i=0; i<_DFSList.size(); ++i){
		r = 0;
		tmpPtr = _gate.find(_DFSList[i])->second;
		if(tmpPtr->_gateType != AIG_GATE) continue;
		GateNode tmpNode(tmpPtr);
		if((gateHash._buckets + gateHash.bucketNum(tmpNode))->size() != 0 && (tmpNode += (*(gateHash._buckets + gateHash.bucketNum(tmpNode)))[0])){
			for(size_t j=0; j<(gateHash._buckets + gateHash.bucketNum(tmpNode))->size(); ++j){
				if((tmpNode += (*(gateHash._buckets + gateHash.bucketNum(tmpNode)))[j])){
					replaceFanoutOfAIGFanin(tmpPtr, (*(gateHash._buckets + gateHash.bucketNum(tmpNode)))[0].gatePtr()->_gateId);
					replaceFaninOfAIGFanout(tmpPtr, (*(gateHash._buckets + gateHash.bucketNum(tmpNode)))[0].gatePtr()->_gateId);
					--_header[4];
					vector<unsigned>::iterator it = _gateId.begin();
					while(*it != tmpPtr->_gateId) ++it;
					_gateId.erase(it);
					_gate.erase(tmpPtr->_gateId);
					r = 1;
				}
			}
		}
		if(r == 0){
			gateHash.insert(tmpNode);
		}
	}
	doDFSList();
	return;
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirMgr::replaceFanoutOfAIGFanin(CirGate* ptr, unsigned& setFanout)
{
	CirGate* setPtr = NULL;
	vector<unsigned>::iterator it;
	for(int i=0; i<2; ++i){
		setPtr = _gate.find(ptr->_fanin[i]/2)->second;
		it = setPtr->_fanout.begin();
		while(it != setPtr->_fanout.end()){
			if(*it == ptr->_gateId*2 + ptr->_fanin[i]%2){
				setPtr->_fanout.erase(it);
				break;
			}
			++it;
		}
		setPtr->_fanout.push_back(setFanout*2 + ptr->_fanin[i]%2);
	}
	return;
}
void
CirMgr::replaceFaninOfAIGFanout(CirGate* ptr, unsigned& setFanin)
{
	CirGate* setPtr = NULL;
	for(size_t i=0; i<ptr->_fanout.size(); ++i){
		setPtr = _gate.find(ptr->_fanout[i]/2)->second;
		for(int j=0; j<2; ++j){
			if(setPtr->_fanin[j] == ptr->_gateId*2 + ptr->_fanout[i]%2){
				setPtr->_fanin[j] = setFanin*2 + ptr->_fanout[i]%2;
				break;
			}
		}
	}
}








