/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
  friend class CirMgr;
  friend class GateNode;
  friend class FecGroup;
  CirGate(const unsigned& i, const GateType& t, const unsigned& l) : _gateType(t), _gateId(i), _lineNo(l), _inDFS(0), _DFSRef(0), _printRef(0), _simValue(0), _fecIdx(0) { _gateName = NULL; }
  ~CirGate(){
    if(_gateName != NULL) delete _gateName;
  }

  // Basic access methods
  string getTypeStr() const;
  unsigned getLineNo() const { return _lineNo; }
  bool isAig() const { if(_gateType == AIG_GATE) return true; return false; }

   // Printing functions
  void printGate() const;
  void reportGate() const;
  void reportFanin(int level) const;
  void reportFanout(int level) const;

  bool sameSim(CirGate* ptr){
    for(size_t i=0; i<_simValues.size(); ++i){
      if(_simValues[i] != ptr->_simValues[i]) return false;
    }
    return true;
  }
   
private:
  GateType _gateType;
  string* _gateName;
  unsigned _lineNo;
  unsigned _gateId;
  unsigned _fanin[2];
  unsigned _DFSRef;
  unsigned _simValue;
  unsigned _fecIdx;
  vector<unsigned> _simValues;
  mutable unsigned _printRef;
  vector<unsigned> _fanout;
  bool _inDFS;
  void myReportFanin(int level, int n, bool inv = false) const;
  void myReportFanout(int level, int n, bool inv = false)const;
};

#endif // CIR_GATE_H
