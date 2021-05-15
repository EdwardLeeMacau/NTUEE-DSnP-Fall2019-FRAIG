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
#include "rnGen.h"

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

   // Get FECGroups
   const vector<vector<CirGate*>>& getFECGroups() const;

   // Member functions about cirMgr
   void reset();

private:
   ofstream           *_simLog;

   // DFSList Maintainer or builder
   void DepthFirstTraversal(const unsigned int, vector<CirGate*> &) const;
   void DepthFirstTraversal(CirGate* , vector<CirGate*> &) const;

   // Loader Function
   bool readHeader(const string&);
   bool loadInput(const unsigned int&);
   bool loadOutput(const unsigned int&, const unsigned int&);
   bool loadAIG(const unsigned int&);
   bool loadLatch(const unsigned int&, const unsigned int&);
   bool loadSymbol(const unsigned int&, const string&);
   bool loadComment(const string&);

   // Mgr Basic Attribute
   void getFloatingList(vector<unsigned int>& /* floating */ );
   void getNotUsedList(vector<unsigned int>& /* notused */ );

   // Gate Property Modifier
   void connect(CirGate*, CirGate* );
   void sortInOut();
   void sortIn();
   void sortOut();

   // Gate Property Modifier (Defined outside cirMgr.cpp)
   bool removeGate(const unsigned int&);
   bool removeGate(CirGate*);
   bool removePIGate(CirGate*);
   bool removePOGate(CirGate*);
   bool removeAIGate(CirGate*);
   bool removeUndefGate(CirGate*);
   void replaceConnection(CirGate*, CirGate*, CirGate*);
   void mergeGate(CirGate* , CirGate* );

   // Gate Property Identifier (Defined outside cirMgr.cpp)
   bool identityFanin(CirGate* , bool& /* phase */) const;
   bool hasConstFanin(CirGate* , int& /* index */) const;
   bool identityStruct(CirGate* , CirGate* ) const;

   // FRAIG Function
   void createCNF(SatSolver& /* solver */);

   // Simulation Pattern
   void encodePattern(const vector<size_t>& /* txPatterns */, const vector<size_t>& /* rxPatterns */, int /* maskLength */, ostream* /* os */) const;
   int parsePattern(vector<vector<size_t>>& /* patternsGroups */, istream& /* is */) const;
   int genPattern(vector<size_t>& /* txPatterns */) const;

   // Simulation Function
   void splitFECGroups(int /* maskLength */, bool /* again */);
   void initFECGroups();
   void simulateOnce(const vector<size_t>& /* txPatterns */, int /* maskLength */, bool /* again */);
   void feedSignal(const vector<size_t>& /* txPatterns */);
   void getSignal(vector<size_t>& /* rxPatterns */);

   // Message Printer
   void SimplifyMsg(CirGate* , CirGate* ) const;
   void StrashMsg(CirGate*, CirGate* ) const;
   void FraigMsg(CirGate*, CirGate* ) const;

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

   vector<vector<CirGate*>> FECs;   // Functionally Equivalent Candidate (FEC) Groups
   RandomNumGen rnGen;              // Random Number Generator
};

#endif // CIR_MGR_H
