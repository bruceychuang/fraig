/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <map>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
  ifstream aigFile;
  aigFile.open(fileName.c_str(), ifstream::in);
  string str;
  string tok;
  int tmpGateId;
  CirGate* tmpGatePtr;
  //header
  getline(aigFile, str);
  ++lineNo;
  size_t pos = myStrGetTok(str, tok, 0, ' ');
  for(int i=0; i<5; ++i){
    pos = myStrGetTok(str, tok, pos, ' ');
    if(!myStr2Int(tok, _header[i])) return parseError(ILLEGAL_NUM);
  }
  //const_gate
  tmpGatePtr = new CirGate(0, CONST_GATE, 0);
  _gate.insert(pair<unsigned, CirGate*>(0, tmpGatePtr));
  _gateId.push_back(0);
  //inputs
  for(int i=0; i < _header[1]; ++i){
    getline(aigFile, str);
    ++lineNo;
    if(!myStr2Int(str, tmpGateId)) return parseError(ILLEGAL_NUM);
    if(tmpGateId == 0) tmpGatePtr = new CirGate(tmpGateId/2, CONST_GATE, lineNo);
    else tmpGatePtr = new CirGate(tmpGateId/2, PI_GATE, lineNo);
    _gate.insert(pair<unsigned, CirGate*>(tmpGateId/2, tmpGatePtr));
    _gateId.push_back(tmpGateId/2);
  }
  //go through outputs
  for(int i=0; i<_header[3]; ++i){
    getline(aigFile, str);
    ++lineNo;
    if(!myStr2Int(str, tmpGateId)) return parseError(ILLEGAL_NUM);
    tmpGatePtr = new CirGate(_header[0]+i+1, PO_GATE, lineNo);
    _gate.insert(pair<unsigned, CirGate*>(_header[0]+i+1, tmpGatePtr));
    _gateId.push_back(_header[0]+i+1);
    tmpGatePtr->_fanin[0] = tmpGateId;
  }
  //define AND gates with ID only
  streampos spos = aigFile.tellg();
  unsigned tmpLineNo = lineNo;
  for(int i=0; i < _header[4]; ++i){
    getline(aigFile, str);
    ++lineNo;
    pos = myStrGetTok(str, tok, 0, ' ');
    if(!myStr2Int(tok, tmpGateId)) return parseError(ILLEGAL_NUM);
    tmpGatePtr = new CirGate(tmpGateId/2, AIG_GATE, lineNo);
    _gate.insert(pair<unsigned, CirGate*>(tmpGateId/2, tmpGatePtr));
    _gateId.push_back(tmpGateId/2);
    for(int j=0; j<2; ++j){
      pos = myStrGetTok(str, tok, pos, ' ');
      if(!myStr2Int(tok, tmpGateId)) return parseError(ILLEGAL_NUM);
      tmpGatePtr->_fanin[j] = tmpGateId;
    }
  }
  //AIG fanout->POs
  map<unsigned, CirGate*>::iterator it;
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    it = _gate.find(_gateId[i]);
    tmpGateId = it->second->_fanin[0];
    it = _gate.find(tmpGateId/2);
    if(it == _gate.end()){
      tmpGatePtr = new CirGate(tmpGateId/2, UNDEF_GATE, _gate.find(_gateId[i])->second->_lineNo);
      _gate.insert(pair<unsigned, CirGate*>(tmpGateId/2, tmpGatePtr));
      _gateId.push_back(tmpGateId/2);
      it = _gate.find(tmpGateId/2);
    }
    it->second->_fanout.push_back(2*_gateId[i]+tmpGateId%2);
  }
  //complete AND gates
  aigFile.seekg(spos);
  lineNo = tmpLineNo;
  int tmpGateId2;
  for(int i=0; i < _header[4]; ++i){
    getline(aigFile, str);
    ++lineNo;
    pos = myStrGetTok(str, tok, 0, ' ');
    if(!myStr2Int(tok, tmpGateId)) return parseError(ILLEGAL_NUM);
    for(int j=0; j<2; ++j){
      pos = myStrGetTok(str, tok, pos, ' ');
      if(!myStr2Int(tok, tmpGateId2)) return parseError(ILLEGAL_NUM);
      it = _gate.find(tmpGateId2/2);
      if(it == _gate.end()){
        tmpGatePtr = new CirGate(tmpGateId2/2, UNDEF_GATE, lineNo);
        _gate.insert(pair<unsigned, CirGate*>(tmpGateId2/2, tmpGatePtr));
        _gateId.push_back(tmpGateId2/2);
      }
      else it->second->_fanout.push_back(tmpGateId+tmpGateId2%2);
    }
  }
  //setting gate name
  getline(aigFile, str);
  while(str[0] == 'i'){
    pos = myStrGetTok(str, tok, 1, ' ');
    if(!myStr2Int(tok, tmpGateId)) return parseError(ILLEGAL_NUM);
    myStrGetTok(str, tok, pos, ' ');
    _gate.find(_gateId[tmpGateId+1])->second->_gateName = new string(tok);
    getline(aigFile, str);
  }
  while(str[0] == 'o'){
    pos = myStrGetTok(str, tok, 1, ' ');
    if(!myStr2Int(tok, tmpGateId)) return parseError(ILLEGAL_NUM);
    myStrGetTok(str, tok, pos, ' ');
    _gate.find(_gateId[tmpGateId+1+_header[1]])->second->_gateName = new string(tok);
    getline(aigFile, str);
  }
  aigFile.close();
  //finding floating gates
  doFloList();
  //DFSList
  doDFSList();
  //Debug------------------------------------
  /*cout<<"-----DFS-----"<<endl;
  for(size_t i=0; i<_DFSList.size(); ++i){
    cout<<_DFSList[i]<<endl;
  }
  cout<<"-------------"<<endl;
  *///-----------------------------------------

  //Debug
  /*
  for(int i=0; i<_floGateId1.size(); ++i){
    cout<<"f fin : "<<_floGateId1[i]<<endl;
  }
  for(int i=0; i<_floGateId2.size(); ++i){
    cout<<"unused : "<<_floGateId2[i]<<endl;
  }

  
  for(int i=0; i<5; ++i){
    cout<<"header["<<i<<"] = "<<_header[i]<<endl;
  }
  for(int i=0; i<_gateId.size(); ++i){
    cout<<"gateId : "<<_gateId[i]<<" from :"<<_gate.find(_gateId[i])->second->_fanin[0]<<" & "<<_gate.find(_gateId[i])->second->_fanin[1];
    cout<<" / gateType : "<<_gate.find(_gateId[i])->second->getTypeStr();
    cout<<" / in line ("<<_gate.find(_gateId[i])->second->getLineNo();
    cout<<" / to :";
    for(int j=0; j<_gate.find(_gateId[i])->second->_fanout.size(); j++){
      cout<<_gate.find(_gateId[i])->second->_fanout[j]<<" ";
    }
    cout<<endl;
  }
  */
  return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
  cout<<"Circuit Statistics"<<endl;
  cout<<"=================="<<endl;
  cout<<"  PI"<<setw(12)<<right<<_header[1]<<endl;
  cout<<"  PO"<<setw(12)<<_header[3]<<endl;
  cout<<"  AIG"<<setw(11)<<_header[4]<<endl;
  cout<<"------------------"<<endl;
  cout<<"  Total"<<setw(9)<<_header[1]+_header[3]+_header[4]<<endl;
}

void
CirMgr::printNetlist() const
{
  int num=0;
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    myPrintNetlist(_gate.find(_gateId[i])->second, num);
  }
  ++_printRefGlobal;
/*
   cout << endl;
   for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGate();
   }
*/
}

void
CirMgr::printPIs() const
{
  cout << "PIs of the circuit:";
  for(int i=1; i<1+_header[1]; ++i){
    cout<<" "<<_gateId[i];
  }
  cout<<endl;
}

void
CirMgr::printPOs() const
{
  cout << "POs of the circuit:";
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    cout<<" "<<_gateId[i];
  }
  cout<<endl;
}

void
CirMgr::printFloatGates() const
{
  if(_floGateId1.size() != 0) cout<<"Gates with floating fanin(s):";
  for(size_t i=0; i<_floGateId1.size(); ++i){
    cout<<" "<<_floGateId1[i];
  }
  if(_floGateId1.size() != 0) cout<<endl;
  if(_floGateId2.size() != 0) cout<<"Gates defined but not used  :";
  for(size_t i=0; i<_floGateId2.size(); ++i){
    cout<<" "<<_floGateId2[i];
  }
  if(_floGateId2.size() != 0) cout<<endl;
}

void
CirMgr::printFECPairs()
{
  unsigned posValue = 0;
  for(size_t i=0; i<_fecGroups.size(); ++i){
    cout<<"["<<i<<"] ";
    posValue = _fecGroups[i]._fecGate[0]->_simValue;
    for(size_t j=0; j<_fecGroups[i].size(); ++j){
      if(_fecGroups[i][j]->_simValue != posValue) cout<<"!";
      cout<<_fecGroups[i][j]->_gateId<<" ";
    }
    cout<<endl;
  }
}

void
CirMgr::writeAag(ostream& outfile) const
{
  //header
  outfile<<"aag";
  for(int i=0; i<5; ++i){
    outfile<<" "<<_header[i];
  }
  outfile<<endl;
  //inputs
  for(int i=1; i<1+_header[1]; ++i){
    outfile<<2*_gateId[i]<<endl;
  }
  //outputs
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    outfile<<_gate.find(_gateId[i])->second->_fanin[0]<<endl;
  }
  //AIG gates
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    myWrite(_gate.find(_gateId[i])->second);
  }
  ++_printRefGlobal;
  //inputs' names 
  for(int i=1; i<1+_header[1]; ++i){
    map<unsigned, CirGate*>::const_iterator it = _gate.find(_gateId[i]);
    if(it->second->_gateName != NULL) outfile<<"i"<<i-1<<" "<<*(it->second->_gateName)<<endl;
  }
  //outputs' names
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    map<unsigned, CirGate*>::const_iterator it = _gate.find(_gateId[i]);
    if(it->second->_gateName != NULL) outfile<<"o"<<i-1-_header[1]<<" "<<*(it->second->_gateName)<<endl;
  }
  return;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
}
//Private funtions
void
CirMgr::doFloList()
{
  CirGate* tmpGatePtr = NULL;
  _floGateId1.clear();
  _floGateId2.clear();
  for(int i=1; i<1+_header[1]; ++i){
    tmpGatePtr = _gate.find(_gateId[i])->second;
    if(_gate.find(_gateId[i])->second->_fanout.size() == 0) _floGateId2.push_back(_gateId[i]);
    if(tmpGatePtr->_fanout.size() == 0) _floGateId2.push_back(_gateId[i]);
  }
  for(int i=1+_header[1]; i<1+_header[1]+_header[3]; ++i){
    tmpGatePtr = _gate.find(_gateId[i])->second;
    if(_gate.find(tmpGatePtr->_fanin[0]/2)->second->_gateType == UNDEF_GATE) _floGateId1.push_back(_gateId[i]);
  }
  for(int i=1+_header[1]+_header[3]; i<1+_header[1]+_header[3]+_header[4]; ++i){
    tmpGatePtr = _gate.find(_gateId[i])->second;
    if(_gate.find(tmpGatePtr->_fanin[0]/2)->second->_gateType == UNDEF_GATE) _floGateId1.push_back(_gateId[i]);
    else if(_gate.find(tmpGatePtr->_fanin[1]/2)->second->_gateType == UNDEF_GATE) _floGateId1.push_back(_gateId[i]);
    if(tmpGatePtr->_fanout.size() == 0) _floGateId2.push_back(_gateId[i]);
  }
  sort(_floGateId1.begin(), _floGateId1.end());
  sort(_floGateId2.begin(), _floGateId2.end());
}
void
CirMgr::doDFSList()
{
  _DFSList.clear();
  ++_DFSRefGlobal;
  for(int i=1+_header[1];  i<1+_header[1]+_header[3]; ++i){
    doDFSList(_gate.find(_gateId[i])->second);
  }
}
void
CirMgr::doDFSList(CirGate* ptr)
{
  if(ptr->_DFSRef == _DFSRefGlobal) return;
  else if(ptr->_gateType == PO_GATE) doDFSList(_gate.find(ptr->_fanin[0]/2)->second);
  else if(ptr->_gateType == AIG_GATE){
    doDFSList(_gate.find(ptr->_fanin[0]/2)->second);
    doDFSList(_gate.find(ptr->_fanin[1]/2)->second);
  }
  _DFSList.push_back(ptr->_gateId);
  ptr->_DFSRef = _DFSRefGlobal;
  return;
}
void
CirMgr::myDFS(CirGate* ptr)
{
  ptr->_inDFS = 1;
  if(ptr->_gateType == PO_GATE){
    myDFS(_gate.find(ptr->_fanin[0]/2)->second);
  }
  else if(ptr->_gateType == AIG_GATE){
    myDFS(_gate.find(ptr->_fanin[0]/2)->second);
    myDFS(_gate.find(ptr->_fanin[1]/2)->second); 
  }
  return;
}
void
CirMgr::myPrintNetlist(CirGate* ptr, int& num) const
{
  if(ptr->_printRef == _printRefGlobal) return;
  else if(ptr->_gateType == PI_GATE){
    cout<<"["<<num<<"] ";
    ++num;
    cout<<setw(4)<<left<<ptr->getTypeStr()<<ptr->_gateId<<" ";
    if(ptr->_gateName != NULL) cout<<"("<<*(ptr->_gateName)<<") ";
    cout<<endl;
    ptr->_printRef = _printRefGlobal;
  }
  else if(ptr->_gateType == PO_GATE){
    myPrintNetlist(_gate.find(ptr->_fanin[0]/2)->second, num);
    cout<<"["<<num<<"] ";
    ++num;
    cout<<setw(4)<<left<<ptr->getTypeStr()<<ptr->_gateId<<" ";
    if(_gate.find(ptr->_fanin[0]/2)->second->_gateType == UNDEF_GATE) cout<<"*";
    if(ptr->_fanin[0]%2) cout<<"!";
    cout<<ptr->_fanin[0]/2<<" ";
    if(ptr->_gateName != NULL) cout<<"("<<*(ptr->_gateName)<<") ";
    cout<<endl;
    ptr->_printRef = _printRefGlobal;
  }
  else if(ptr->_gateType == AIG_GATE){
    myPrintNetlist(_gate.find(ptr->_fanin[0]/2)->second, num);
    myPrintNetlist(_gate.find(ptr->_fanin[1]/2)->second, num);
    cout<<"["<<num<<"] ";
    ++num;
    cout<<setw(4)<<left<<ptr->getTypeStr()<<ptr->_gateId<<" ";
    if(_gate.find(ptr->_fanin[0]/2)->second->_gateType == UNDEF_GATE) cout<<"*";
    if(ptr->_fanin[0]%2) cout<<"!";
    cout<<ptr->_fanin[0]/2<<" ";
    if(_gate.find(ptr->_fanin[1]/2)->second->_gateType == UNDEF_GATE) cout<<"*";
    if(ptr->_fanin[1]%2) cout<<"!";
    cout<<ptr->_fanin[1]/2<<endl;
    ptr->_printRef = _printRefGlobal;
  }
  else if(ptr->_gateType == CONST_GATE){
    cout<<"["<<num<<"] CONST0"<<endl;
    ++num;
    ptr->_printRef = _printRefGlobal;
  }
  return;
}
void
CirMgr::myWrite(CirGate* ptr) const
{
  if(ptr->_printRef == _printRefGlobal) return;
  else if(ptr->_gateType == PO_GATE){
    myWrite(_gate.find(ptr->_fanin[0]/2)->second);
  }
  else if(ptr->_gateType == AIG_GATE){
    myWrite(_gate.find(ptr->_fanin[0]/2)->second);
    myWrite(_gate.find(ptr->_fanin[1]/2)->second);
    cout<<2*ptr->_gateId<<" ";
    cout<<ptr->_fanin[0]<<" ";
    cout<<ptr->_fanin[1]<<endl;
    ptr->_printRef = _printRefGlobal;
  }
}
