/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <algorithm>
#include <vector>
#include <stack>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr() {}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const;

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
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   // Member functions about cirMgr
   void reset();

private:
   ofstream           *_simLog;

   void DepthFirstTraversal(const unsigned int, vector<CirGate*> &) const;
   void DepthFirstTraversal(CirGate* , vector<CirGate*> &) const;

   bool readHeader(const string&);
   bool loadInput(const unsigned int&);
   bool loadOutput(const unsigned int&, const unsigned int&);
   bool loadAIG(const unsigned int&);
   bool loadLatch(const unsigned int&, const unsigned int&);
   bool loadSymbol(const unsigned int&, const string&);
   bool loadComment(const string&);
   
   vector<unsigned int> _pin;       // PinIn Number
   vector<unsigned int> _pout;      // PinOut Number
   vector<unsigned int> _aig;       // AIGs Number
   vector<unsigned int> _latch;     // Latch Number
   vector<unsigned int> _floating;  // Floating Gates
   vector<unsigned int> _notused;   // Not in used Gates
   vector<CirGate*>     _gates;

   stringstream _comment;           // Store the comment

   unsigned int _M;                 // Maximal Variable Index
   unsigned int _I;                 // Number of Inputs
   unsigned int _L;                 // Number of Latches
   unsigned int _O;                 // Number of Outputs
   unsigned int _A;                 // Number of AND Gates
};

#endif // CIR_MGR_H
