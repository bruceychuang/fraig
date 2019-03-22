/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/
unsigned T=50;
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
	vector<unsigned> pattern;
	for(int j=0; j<_header[1]; ++j){
		pattern.push_back(rnGen(INT_MAX));
	}
	if(_simed == 0) mySim1(pattern);
	for(int i=0; i<T; ++i){
		pattern.clear();
		for(int j=0; j<_header[1]; ++j){
			pattern.push_back(rnGen(INT_MAX));
		}
		mySim2(pattern);
	}
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	unsigned counter = 0;
	vector<unsigned> pattern;
	for(int i=0; i<_header[1]; ++i){
		pattern.push_back(0);
	}
	while(!patternFile.eof()){
		string s;
		patternFile>>s;
		if(s == "") break;
		for(int i=0; i<_header[1]; ++i){
			if(s[i] == '1') pattern[i] += 1<<counter;
			else if(s[i] != '0' && s[i] != '\n') cerr<<"Error: Pattern("<<s<<") contains a non-0/1 character('"<<s[i]<<"').";
		}
		++counter;
		if(counter == 32){
			if(_simed == 0) mySim1(pattern);
			else mySim2(pattern);
			counter = 0;
			for(int i=0; i<_header[1]; ++i){
				pattern[i] = 0;
			}
		}
	}
	/*
	CirGate* p;
	for(int i=1; i<1+_header[1]; ++i){
		p = _gate.find(_gateId[i])->second;
		cout<<"gate : "<<p->_gateId<<" simvalue : ";
		for(int j=0; j<32; ++j){
			cout<<((p->_simValue)>>j)%2;
		}
		cout<<endl;
	}
	*/
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void
CirMgr::mySim1(vector<unsigned>& pattern)
{
	HashSet<FecGroup> simHash(getHashSize(_DFSList.size()));
	CirGate* ptr = NULL;
	FecGroup tmpF(_gate.find(0)->second);
	simHash.insert(tmpF);
	for(int i=0; i<_header[1]; ++i){
		_gate.find(_gateId[i+1])->second->_simValue = pattern[i];
	}
	bool b;
	for(size_t i=0; i<_DFSList.size(); ++i){
		b=0;
		ptr = _gate.find(_DFSList[i])->second;
		if(ptr->_gateType == UNDEF_GATE) ptr->_simValue = 0;
		else if(ptr->_gateType == AIG_GATE){
			if(ptr->_fanin[0]%2 == 0 && ptr->_fanin[1]%2 == 0) ptr->_simValue = (_gate.find(ptr->_fanin[0]/2)->second->_simValue) & (_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			else if(ptr->_fanin[0]%2 == 0 && ptr->_fanin[1]%2 == 1) ptr->_simValue = (_gate.find(ptr->_fanin[0]/2)->second->_simValue) & ~(_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			else if(ptr->_fanin[0]%2 == 1 && ptr->_fanin[1]%2 == 0) ptr->_simValue = ~(_gate.find(ptr->_fanin[0]/2)->second->_simValue) & (_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			else if(ptr->_fanin[0]%2 == 1 && ptr->_fanin[1]%2 == 1) ptr->_simValue = ~(_gate.find(ptr->_fanin[0]/2)->second->_simValue) & ~(_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			ptr->_simValues.push_back(ptr->_simValue);
		}
		for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
			CirGate* ptr2 = _gate.find(_gateId[i])->second;
			if(ptr2->_fanin[0]%2 == 0) ptr2->_simValue = _gate.find(ptr2->_fanin[0]/2)->second->_simValue;
			else ptr2->_simValue = ~(_gate.find(ptr2->_fanin[0]/2)->second->_simValue);
		}
		tmpF.reset(ptr);
		vector<FecGroup>* p1 = (simHash._buckets+((tmpF._simValue))%simHash._numBuckets);
		vector<FecGroup>* p2 = (simHash._buckets+(~(tmpF._simValue))%simHash._numBuckets);
		if(p1->size() != 0 || p2->size() != 0){
			for(size_t j=0; j<p1->size(); ++j){
				if((*p1)[j] == tmpF){
					(*p1)[j].merge(tmpF);
					b = 1;
					break;
				}
			}
			for(size_t j=0; j<p2->size(); ++j){
				if((*p2)[j] == tmpF && b == 0){
					(*p2)[j].merge(tmpF);
					b = 1;
					break;
				}
			}
		}
		if(b == 0) simHash.insert(tmpF);
	}
	for(size_t i=0; i<simHash._numBuckets; ++i){
		for(size_t j=0; j<(simHash._buckets+i)->size(); ++j){
			if((*(simHash._buckets+i))[j].size() > 1){
				_fecGroups.push_back((*(simHash._buckets+i))[j]);
				for(size_t k=0; k<(*(simHash._buckets+i))[j].size(); ++k){
					(*(simHash._buckets+i))[j][k]->_fecIdx = _fecGroups.size();
				}
			}
		}
	}
	myLog();
	_simed = 1;
	return;
}
void
CirMgr::mySim2(vector<unsigned>& pattern)
{
	CirGate* ptr = NULL;
	for(int i=0; i<_header[1]; ++i){
		_gate.find(_gateId[i+1])->second->_simValue = pattern[i];
	}
	for(size_t i=0; i<_DFSList.size(); ++i){
		ptr = _gate.find(_DFSList[i])->second;
		if(ptr->_gateType == UNDEF_GATE) ptr->_simValue = 0;
		else if(ptr->_gateType == AIG_GATE){
			if(ptr->_fanin[0]%2 == 0 && ptr->_fanin[1]%2 == 0) ptr->_simValue = (_gate.find(ptr->_fanin[0]/2)->second->_simValue) & (_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			else if(ptr->_fanin[0]%2 == 0 && ptr->_fanin[1]%2 == 1) ptr->_simValue = (_gate.find(ptr->_fanin[0]/2)->second->_simValue) & ~(_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			else if(ptr->_fanin[0]%2 == 1 && ptr->_fanin[1]%2 == 0) ptr->_simValue = ~(_gate.find(ptr->_fanin[0]/2)->second->_simValue) & (_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			else if(ptr->_fanin[0]%2 == 1 && ptr->_fanin[1]%2 == 1) ptr->_simValue = ~(_gate.find(ptr->_fanin[0]/2)->second->_simValue) & ~(_gate.find(ptr->_fanin[1]/2)->second->_simValue);
			ptr->_simValues.push_back(ptr->_simValue);
		}
	}
	/*
	for(size_t i=0; i<_DFSList.size(); ++i){
		ptr = _gate.find(_DFSList[i])->second;
		cout<<"Gate : "<<ptr->_gateId<<" SimValue : ";
		for(int j=0; j<32; ++j){
			cout<<((ptr->_simValue)>>j)%2;
		}
		cout<<endl;
	}
	*/
	for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
		ptr = _gate.find(_gateId[i])->second;
		if(ptr->_fanin[0]%2 == 0) ptr->_simValue = _gate.find(ptr->_fanin[0]/2)->second->_simValue;
		else ptr->_simValue = ~(_gate.find(ptr->_fanin[0]/2)->second->_simValue);
	}
	myLog();
	size_t oldFecSize = _fecGroups.size();
	for(size_t i=0; i<oldFecSize; ++i){
		HashSet<FecGroup> simHash(getHashSize(_fecGroups[i].size()));
		FecGroup tmpF(_gate.find(0)->second);
		bool b = 0;
		for(size_t j=0; j<_fecGroups[i].size(); ++j){
			b=0;
			tmpF.reset(_fecGroups[i][j]);
			vector<FecGroup>* p1 = (simHash._buckets+((tmpF._simValue))%simHash._numBuckets);
			vector<FecGroup>* p2 = (simHash._buckets+(~(tmpF._simValue))%simHash._numBuckets);
			if(p1->size() != 0 || p2->size() != 0){
				for(size_t k=0; k<p1->size(); ++k){
					if((*p1)[k] == tmpF){
						(*p1)[k].merge(tmpF);
						b = 1;
						break;
					}
				}
				for(size_t k=0; k<p2->size(); ++k){
					if((*p2)[k] == tmpF && b == 0){
						(*p2)[k].merge(tmpF);
						b = 1;
						break;
					}
				}
			}
			if(b == 0) simHash.insert(tmpF);
		}
		for(size_t l=0; l<simHash._numBuckets; ++l){
			for(size_t j=0; j<(simHash._buckets+l)->size(); ++j){
				if((*(simHash._buckets+l))[j].size() > 1){
					_fecGroups.push_back((*(simHash._buckets+l))[j]);
					for(size_t k=0; k<(*(simHash._buckets+l))[j].size(); ++k){
						(*(simHash._buckets+l))[j][k]->_fecIdx = _fecGroups.size() - oldFecSize;
					}
				}
			}
		}
	}
	vector<FecGroup>::iterator it = _fecGroups.begin();
	for(size_t i=0; i<oldFecSize; ++i){
		it = _fecGroups.begin();
		_fecGroups.erase(it);
	}
}
void
CirMgr::myLog()
{	
	CirGate* ptr = NULL;
	if(_simLog == NULL) return;
	for(int i=0; i<32; ++i){
		for(size_t j=1; j<1+_header[1]; ++j){
			(*_simLog)<<(((_gate.find(_gateId[j])->second->_simValue)>>i)%2);
		}
		(*_simLog)<<" ";
		for(size_t j=1+_header[1]; j<1+_header[1]+_header[3]; ++j){
			ptr = _gate.find(_gateId[j])->second;
			if(ptr->_fanin[0]%2 == 0) (*_simLog)<<(((_gate.find(ptr->_fanin[0]/2)->second->_simValue)>>i)%2);
			else (*_simLog)<<(((~(_gate.find(ptr->_fanin[0]/2)->second->_simValue))>>i)%2);
			
		}
		(*_simLog)<<endl;
	}
	return;
}








