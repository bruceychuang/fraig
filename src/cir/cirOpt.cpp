/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
  for(int i=1+_header[1];  i<1+_header[1]+_header[3]; ++i){
    myDFS(_gate.find(_gateId[i])->second);
  }
  vector<unsigned>::iterator vit = _gateId.begin();
  vector<unsigned>::iterator vit2;
  map<unsigned, CirGate*>::iterator mit;
  while(vit != _gateId.end()){
  	mit = _gate.find(*vit);
  	if(mit->second->_gateType == AIG_GATE || mit->second->_gateType == UNDEF_GATE){
  		if(mit->second->_inDFS == false){
  			if(mit->second->_gateType == AIG_GATE){
  				--_header[4];
  				deleteFanoutOfAIGFanin(mit->second);
  			}
  			_gateId.erase(vit);
  			_gate.erase(mit);
  			--vit;
  		}
  	}
  	++vit;
  }
  doFloList();
  return;
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
	vector<unsigned>::iterator vit = _DFSList.begin();
	vector<unsigned>::iterator vit2;
	map<unsigned, CirGate*>::iterator mit;
	CirGate* ptr = NULL;
	while(vit != _DFSList.end()){
		vit2 = _gateId.begin();
		while(*vit2 != *vit) ++vit2;
		mit = _gate.find(*vit);
		ptr = mit->second;
		if(mit->second->_gateType == AIG_GATE){
			if(ptr->_fanin[0] == 0 || ptr->_fanin[1] == 0 || (ptr->_fanin[0]%2 == 0 && ptr->_fanin[1] == ptr->_fanin[0]+1) || (ptr->_fanin[1]%2 == 0 && ptr->_fanin[0] == ptr->_fanin[1]+1)){
				deleteFanoutOfAIGFanin(mit->second);
				setFaninOfAIGFanout(mit->second, 0);
				--_header[4];
				_gateId.erase(vit2);
				_gate.erase(mit);
			}
			else if(ptr->_fanin[0] == 1){
				setFanoutOfAIGFanin(mit->second, ptr->_fanout);
				setFaninOfAIGFanout(mit->second, ptr->_fanin[1]);
				--_header[4];
				_gateId.erase(vit2);
				_gate.erase(mit);
			}
			else if(ptr->_fanin[1] == 1 || ptr->_fanin[0] == ptr->_fanin[1]){
				setFanoutOfAIGFanin(mit->second, ptr->_fanout);
				setFaninOfAIGFanout(mit->second, ptr->_fanin[0]);
				--_header[4];
				_gateId.erase(vit2);
				_gate.erase(mit);
			}
		}
		++vit;
	}
	doDFSList();
	return;
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::deleteFanoutOfAIGFanin(CirGate* ptr)
{
	vector<unsigned>::iterator it;
	CirGate* delPtr = NULL;
	for(int i=0; i<2; ++i){
		delPtr = _gate.find(ptr->_fanin[i]/2)->second;
		if(delPtr == 0) continue;
		it = delPtr->_fanout.begin();
		while(it != delPtr->_fanout.end()){
			if(*it == ptr->_gateId*2+ptr->_fanin[i]%2){
				delPtr->_fanout.erase(it);
				break;
			}
			++it;
		}
	}	
	return;
}
void
CirMgr::setFanoutOfAIGFanin(CirGate* ptr, vector<unsigned>& setFanout){
	CirGate* setPtr = NULL;
	vector<unsigned>::iterator it;
	for(int i=0; i<2; ++i){
		setPtr = _gate.find(ptr->_fanin[i]/2)->second;
		it = setPtr->_fanout.begin();
		while(it != setPtr->_fanout.end()){
			if(*it == ptr->_gateId*2+ptr->_fanin[i]%2){
				setPtr->_fanout.erase(it);
				for(size_t j=0; j<setFanout.size(); ++j){
					if(setFanout[j]%2 == 1 && ptr->_fanin[i]%2 == 1) setPtr->_fanout.push_back(setFanout[j]);
					else setPtr->_fanout.push_back(setFanout[j] + ptr->_fanin[i]%2);
				}
			break;
			}
			++it;
		}
	}
	return;
}
void
CirMgr::setFaninOfAIGFanout(CirGate* ptr, unsigned setFanin)
{
	CirGate* setPtr = NULL;
	for(size_t i=0; i<ptr->_fanout.size(); ++i){
		setPtr = _gate.find(ptr->_fanout[i]/2)->second;
		for(int j=0; j<2; ++j){
			if(setPtr->_fanin[j] == ptr->_gateId*2+ptr->_fanout[i]%2){
				if(setFanin%2 == 1 && ptr->_fanout[i]%2 == 1) setPtr->_fanin[j] = setFanin;
				else setPtr->_fanin[j] = setFanin + ptr->_fanout[i]%2;
			}
		}
	}
	return;
}








