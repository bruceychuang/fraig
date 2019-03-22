/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
string CirGate::getTypeStr() const{
  if(_gateType == UNDEF_GATE) return "UNGEF";
  else if(_gateType == PI_GATE) return "PI";
  else if(_gateType == PO_GATE) return "PO";
  else if(_gateType == AIG_GATE) return "AIG";
  else if(_gateType == CONST_GATE) return "CONST";
  else return "";
}
void CirGate::reportGate() const{
  cout<<"=================================================="<<endl;
  cout<<setw(50)<<'=';
  for(int i=0; i<50; ++i){
    cout<<'\b';
  }
  cout<<"= "<<getTypeStr()<<"("<<_gateId<<")";
  if(_gateName != NULL) cout<<"\""<<*_gateName<<"\"";
  cout<<", line "<<_lineNo<<endl;
  cout<<setw(50)<<'=';
  for(int i=0; i<50; ++i){
    cout<<'\b';
  }
  cout<<"= FECs: ";
  if(_fecIdx != 0){
    for(size_t i=0; i<cirMgr->_fecGroups[_fecIdx-1].size(); ++i){
      if(cirMgr->_fecGroups[_fecIdx-1][i]->_simValue == ~(_simValue)) cout<<"!";
      if(cirMgr->_fecGroups[_fecIdx-1][i]->_gateId != _gateId) cout<<cirMgr->_fecGroups[_fecIdx-1][i]->_gateId<<" ";
    }
  }
  cout<<endl;
  cout<<setw(50)<<'=';
  for(int i=0; i<50; ++i){
    cout<<'\b';
  }
  cout<<"= Value: ";
  for(int j=31; j>=0; --j){
    cout<<(_simValue>>j)%2;
    if(j%4 == 0 && j != 0) cout<<"_";
  }
  cout<<endl;
  cout<<"=================================================="<<endl;
}

void CirGate::reportFanin(int level) const{
   assert (level >= 0);
   myReportFanin(level, 0, false);
   cirMgr->setRef(cirMgr->getRef()+1);
}

void CirGate::reportFanout(int level) const{
   assert (level >= 0);
   myReportFanout(level, 0, false);
   cirMgr->setRef(cirMgr->getRef()+1);
}
//Private functions
void CirGate::myReportFanin(int level, int n, bool inv) const{
  for(int i=0; i<n; ++i) cout<<"  ";
  if(inv == true) cout<<"!";
  cout<<getTypeStr()<<" "<<_gateId;
  //if(_gateName != NULL) cout<<" ("<<*_gateName<<")";
  if(level != 0 && _printRef == cirMgr->getRef()) cout<<" (*)";
  cout<<endl;
  if(level != 0 && _printRef != cirMgr->getRef()){
    if(_gateType == PO_GATE || _gateType == AIG_GATE){
      cirMgr->getGate(_fanin[0]/2)->myReportFanin(level-1, n+1, _fanin[0]%2);
      if(_gateType == AIG_GATE) cirMgr->getGate(_fanin[1]/2)->myReportFanin(level-1, n+1, _fanin[1]%2);
      _printRef = cirMgr->getRef();
    }
  }
}
void CirGate::myReportFanout(int level, int n, bool inv) const{
  for(int i=0; i<n; ++i) cout<<"  ";
  if(inv == true) cout<<"!";
  cout<<getTypeStr()<<" "<<_gateId;
  //if(_gateName != NULL) cout<<" ("<<*_gateName<<")";
  if(level != 0 && _printRef == cirMgr->getRef()) cout<<" (*)";
  cout<<endl;
  if(level != 0 && _printRef != cirMgr->getRef()){
    for(size_t i=0; i<_fanout.size(); ++i){
      cirMgr->getGate(_fanout[i]/2)->myReportFanout(level-1, n+1, _fanout[i]%2);
    }
    if(_fanout.size() != 0) _printRef = cirMgr->getRef();
  }
}
