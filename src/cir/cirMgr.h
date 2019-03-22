/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

class GateNode
{
public:
  GateNode(CirGate* p) : _ptr(p) {}
  ~GateNode(){}

  bool operator == (const GateNode& n) const{
    if(_ptr == n._ptr) return true;
    return false;
  }
  bool operator += (const GateNode& n) const{
    if(_ptr->_fanin[0] == n._ptr->_fanin[0] && _ptr->_fanin[1] == n._ptr->_fanin[1]) return true;
    else if(_ptr->_fanin[0] == n._ptr->_fanin[1] && _ptr->_fanin[1] == n._ptr->_fanin[0]) return true;
    return false;
  }
  size_t operator()() const {
    return _ptr->_fanin[0] + _ptr->_fanin[1];
  }
  CirGate* gatePtr(){ return _ptr; }
  unsigned gateId() const { return _ptr->_gateId; }
private:
  CirGate* _ptr;
};

class FecGroup{
  friend class CirMgr;
public:
  FecGroup(CirGate* ptr){
    _fecGate.push_back(ptr);
    _simValue = ptr->_simValue;
  }
  ~FecGroup(){};

  void reset(CirGate* ptr){
    _fecGate.clear();
    _fecGate.push_back(ptr);
    _simValue = ptr->_simValue;
  }
  void merge(FecGroup f){
    _fecGate.push_back(f._fecGate[0]);
  }
  size_t size(){ return _fecGate.size(); }
  CirGate* operator[](size_t& i){ return _fecGate[i]; }
  bool operator==(const FecGroup f){
    if(_simValue == f._simValue) return true;
    else if(~_simValue == f._simValue) return true;
    return false;
  }
  unsigned operator()() const { return _simValue; }
private:
  vector<CirGate*> _fecGate;
  unsigned _simValue;
};


class CirMgr
{
  friend class CirGate;
public:
  CirMgr() : _printRefGlobal(1), _simed(0) {
    for(int i=0; i<5; ++i){
      _header[i]=0;
    }
  }
  ~CirMgr(){
    while(_gate.size() != 0){
      map<unsigned, CirGate*>::iterator it = _gate.begin();
      delete it->second;
      _gate.erase(it);
    } 
  }

  // Access functions
  // return '0' if "gid" corresponds to an undefined gate.
  CirGate* getGate(unsigned gid) const{ return _gate.find(gid)->second; }
  unsigned getRef() const{ return _printRefGlobal;}
  void setRef(unsigned i){ _printRefGlobal = i; return; }

  // Member functions about circuit construction
  bool readCircuit(const string&);

  // Member functions about circuit optimization
  void sweep();
  void optimize();

  // Member functions about simulation
  void randomSim();
  void fileSim(ifstream&);
  void setSimLog(ofstream *logFile) { _simLog = logFile; }

  // Member functions about fraig
  void strash();
  void printFEC() const;
  void fraig();

  // Member functions about circuit reporting
  void printSummary() const;
  void printNetlist() const;
  void printPIs() const;
  void printPOs() const;
  void printFloatGates() const;
  void printFECPairs();
  void writeAag(ostream&) const;
  void writeGate(ostream&, CirGate*) const;

private:
  ofstream           *_simLog;
  //_maxIdx(0), _inNum(1), _latNum(2), _outNum(3), _gateNum(4)
  int _header[5];
  vector<unsigned> _gateId;
  vector<unsigned> _floGateId1; //with floating fanin(s)
  vector<unsigned> _floGateId2; //defined but not used
  vector<unsigned> _DFSList;
  vector<FecGroup> _fecGroups;
  map<unsigned, CirGate*> _gate;
  mutable unsigned _printRefGlobal;
  unsigned _DFSRefGlobal;
  bool _simed;
  void doFloList();
  void doDFSList();
  void doDFSList(CirGate* ptr);
  void myDFS(CirGate* ptr);
  void myPrintNetlist(CirGate* ptr, int& num) const;
  void myWrite(CirGate* ptr) const;
  void deleteFanoutOfAIGFanin(CirGate*);
  void setFanoutOfAIGFanin(CirGate*, vector<unsigned>&);
  void setFaninOfAIGFanout(CirGate*, unsigned);
  void replaceFanoutOfAIGFanin(CirGate* , unsigned&);
  void replaceFaninOfAIGFanout(CirGate* , unsigned&);
  void mySim1(vector<unsigned>&);
  void mySim2(vector<unsigned>&);
  void myLog();
};

#endif // CIR_MGR_H
