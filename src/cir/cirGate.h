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

#define NEG 0x1

using namespace std;

class CirGate;       // Abstract class
class CirConstGate;  // Inheritance
class CirUndefGate;  // Inheritance
class CirPIGate;     // Inheritance
class CirPOGate;     // Inheritance

/**************************************/
/*   Public Function for cirGate      */
/**************************************/

bool isInv(size_t c);
bool isInv(CirGate* c);
unsigned int getNum(CirGate* c);
CirGate* setNum(size_t n);
CirGate* setNum(unsigned int n);
CirGate* setInv(size_t c);
CirGate* setInv(CirGate* c);
CirGate* gate(size_t c);
CirGate* gate(CirGate* c);

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------

class CirGate;
class CirPIGate;
class CirPOGate;
class CirAIGate;
class CirUndefGate;
class CirConstGate;
class CirGateHash;

// TODO: Define your own data members and member functions, or classes

class CirGate
{
public:

   friend class CirMgr;
   friend class CirGateHash;

   CirGate()
      { }
   CirGate(const unsigned int &id, const unsigned int &no)
      : _lineno(no), _gateId(id), _state(0) { }
   virtual ~CirGate()
      { }

   // Basic access methods
   virtual string getTypeStr() const = 0;
   unsigned int getLineNo() const { return _lineno; }
   virtual bool isAig() const { return false; }

   // Printing functions
   virtual void printGate() const = 0;
   virtual void reportGate() const;
   virtual void reportFanin(int level);
   virtual void reportFanout(int level);

   // Simulation Function
   virtual void operate();
   virtual const size_t& getState();
   virtual void setState(const size_t& /* state */);

private:
protected:

   virtual void reportFanin(int level, int indent, bool invert);
   virtual void reportFanout(int level, int indent, bool invert);

   // Basic Property
   virtual bool isFloating() { return false; }
   virtual bool isConst() { return false; }

   // Traversal Property
   static void raiseGlobalMarker() { ++_globalMarker; }
   bool hasSymbol() { return _symbol != ""; }
   bool isMarked() { return _marker == _globalMarker; }
   void mark() { _marker = _globalMarker; }

   // Connection Property
   void addFanin(CirGate* c, bool isInv);
   void addFanout(CirGate* c, bool isInv);
   void disconnectFanin(CirGate* );
   void disconnectFanout(CirGate* );
   void disconnect();

   // FRAIG Property
   void setVar(const Var& /* var*/);
   Var getVar();

   static unsigned int _globalMarker;     // Design for Graph serach Algorithm

   unsigned int _lineno;                  // Where the gate was defined
   unsigned int _marker;                  // Design for Graph search Algorithm
   unsigned int _gateId;                  // Gate ID
   string _symbol;                        // AIGER Symbol
   vector<CirGate*> _fanin;
   vector<CirGate*> _fanout;

   Var _var;                              // SAT verification.
   size_t _state;                         // Simulation Signal.
};

class CirConstGate : public CirGate
{
public:
   CirConstGate() : CirGate(0, 0) {}
   ~CirConstGate() {}

   // Basic access methods
   string getTypeStr() const { return "CONST"; }
   bool isConst() { return true; }

   // Print function
   void printGate() const;                // Self Printing Format
   // void reportFanin(int level) const {};  // None of Fanin

   void operate();
   const size_t& getState();
};

class CirUndefGate : public CirGate
{
public:
   CirUndefGate(const unsigned int &id) : CirGate(id, 0) {}
   ~CirUndefGate() {}

   // Basic access methods
   string getTypeStr() const { return "UNDEF"; }

   // Special Designed For CirUndefGate
   bool isFloating() { return true; }

   // Print function
   void printGate() const;
   // void reportFanin(int level) const {};
   // void reportFanout(int level) const {};

   // void operate();
   // const size_t& getState();
};

class CirPIGate : public CirGate
{
public:
   CirPIGate(const unsigned int &id, const unsigned int &no) : CirGate(id, no) {}
   ~CirPIGate() {}

   // Basic access methods
   string getTypeStr() const { return "PI"; }

   // Print function
   void printGate() const;                // Self Printing Format
   // void reportFanin(int level) const {};  // None of Fanin

};

class CirPOGate : public CirGate
{
public:
   CirPOGate(const unsigned int &id, const unsigned int &no) : CirGate(id, no) { }
   ~CirPOGate() {}

   // Basic access methods
   string getTypeStr() const { return "PO"; }

   // Print function
   void printGate() const;                // Self Printing Format
   // void reportFanout(int level) const {}; // None of Fanout

   // Simulation function
   void operate();
};

class CirAIGate : public CirGate
{
public:
   CirAIGate(const unsigned int &id, const unsigned int &no) : CirGate(id, no) {}
   ~CirAIGate() {}

   // Basic access methods
   string getTypeStr() const { return "AIG"; }

   // Attribute
   bool isAig() const { return true; }

   // Print function
   void printGate() const;

   // Simulation function
   void operate();
};

/*
   DIY Hash function, to check whether 2 CirAIGate has the same input.

   Reference: MathStackExchange
   https://math.stackexchange.com/questions/882877/produce-unique-number-given-two-integers
*/
class CirGateHash
{
public:
   CirGateHash(CirGate* c) : _in0((size_t)c->_fanin[0]), _in1((size_t)c->_fanin[1]) {}
   ~CirGateHash() {}

   size_t operator() () const { return (_in0 + _in1) * (_in0 + _in1 + 1) / 2 + ::min(_in0, _in1); }
   bool operator == (const CirGateHash& k) const { return ((_in0 == k._in0) && (_in1 == k._in1)); }

private:
   size_t _in0, _in1;
};

#endif // CIR_GATE_H
