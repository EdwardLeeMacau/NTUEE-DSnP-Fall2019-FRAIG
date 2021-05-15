/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <iostream>
#include <string>
#include <vector>

#include "cirDef.h"
#include "sat.h"

#define NEG 0x1

using namespace std;

//------------------------------------------------------------------------
//   Public Function for cirGate
//------------------------------------------------------------------------

bool isInv(size_t c);
bool isInv(CirGate *c);
unsigned int getNum(CirGate *c);
CirGate *setNum(size_t n);
CirGate *setNum(unsigned int n);
CirGate *setInv(size_t c);
CirGate *setInv(CirGate *c);
CirGate *gate(size_t c);
CirGate *gate(CirGate *c);

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------

// TODO: Define your own data members and member functions, or classes

class CirGate
{
public:
    friend class CirMgr;
    friend class CirGateHash;

    CirGate() {}
    CirGate(const unsigned int &id, const unsigned int &no) : _lineno(no), _gateId(id), _state(0) {}
    virtual ~CirGate() {}

    // Basic access methods
    virtual string getTypeStr() const = 0;
    unsigned int getLineNo() const
    {
        return _lineno;
    }
    virtual bool isAig() const
    {
        return false;
    }

    virtual void printGate() const = 0;

    /**
     * @brief Print type of the gate, current FECs and output value.
     * @example
     * ================================================================================
     * = PI(1), line 2
     * = FECs:
     * = Value:
     * 00000000_00000000_00000000_00000000_00000000_00000000_00000000_00000000
     * ================================================================================
     */
    virtual void reportGate() const;
    /**
     * @brief Print gate fanin to ostream (cout)
     * @details Gate that have been reported before will simplified with marker (*)
     * @see CirGate::reportFanout()
     */
    virtual void reportFanin(int level);
    /**
     * @brief Print gate fanout to ostream (cout)
     * @details Gate that have been reported before will simplified with marker (*)
     * @see CirGate::reportFanin()
     */
    virtual void reportFanout(int level);

    // Simulation Function
    virtual void operate();
    virtual const size_t &getState();
    virtual void setState(const size_t & /* state */);

protected:
    virtual void reportFanin(int level, int indent, bool invert);
    virtual void reportFanout(int level, int indent, bool invert);

    // Basic Property
    virtual bool isFloating()
    {
        return false;
    }
    virtual bool isConst()
    {
        return false;
    }

    // Traversal Property
    static void raiseGlobalMarker()
    {
        ++_globalMarker;
    }
    bool hasSymbol()
    {
        return _symbol != "";
    }
    bool isMarked()
    {
        return _marker == _globalMarker;
    }
    void mark()
    {
        _marker = _globalMarker;
    }

    // Connection Property
    /**
     * @brief Append CirGate* to _fanin
     * @param[in] c     Fanin in type CirGate*
     * @param[in] isInv If True, store 0x1 at the LSB of CirGate*
     * @see CirGate::disconnectFanin()
     */
    void addFanin(CirGate *c, bool isInv);
    /**
     * @brief Append CirGate* to _fanout
     * @param[in] c     Fanin in type CirGate*
     * @param[in] isInv If True, store 0x1 at the LSB of CirGate*
     * @see CirGate::disconnectFanout()
     */
    void addFanout(CirGate *c, bool isInv);
    /**
     * @brief Disconnect CirGate* from _fanin
     * @param[in] inGate Disconnect 1 if specified, disconnect all if NULL.
     * @see CirGate::addFanin()
     */
    void disconnectFanin(CirGate *);
    /**
     * @brief Disconnect CirGate* from _fanout
     * @param[in] outGate Disconnect 1 if specified, disconnect all if NULL.
     * @see CirGate::addFanout()
     */
    void disconnectFanout(CirGate *);
    /**
     * @brief Disconnect all CirGate* from _fanin and _fanout
     * @see CirGate::disconnectFanin()
     * @see CirGate::disconnectFanout()
     */
    void disconnect();

    // FRAIG Property
    void setVar(const Var & /* var*/);
    Var getVar();

    static unsigned int _globalMarker;  // Design for Graph serach Algorithm

    unsigned int _lineno;  // Where the gate was defined
    unsigned int _marker;  // Design for Graph search Algorithm
    unsigned int _gateId;  // Gate ID
    string _symbol;        // AIGER Symbol
    vector<CirGate *> _fanin;
    vector<CirGate *> _fanout;

    Var _var;       // SAT verification.
    size_t _state;  // Simulation Signal.
};

class CirConstGate : public CirGate
{
public:
    CirConstGate() : CirGate(0, 0) {}
    ~CirConstGate() {}

    // Basic access methods
    string getTypeStr() const
    {
        return "CONST";
    }
    bool isConst()
    {
        return true;
    }

    // Print function
    void printGate() const;  // Self Printing Format
    // void reportFanin(int level) const {};  // None of Fanin

    void operate();
    const size_t &getState();
};

class CirUndefGate : public CirGate
{
public:
    CirUndefGate(const unsigned int &id) : CirGate(id, 0) {}
    ~CirUndefGate() {}

    // Basic access methods
    string getTypeStr() const
    {
        return "UNDEF";
    }

    // Special Designed For CirUndefGate
    bool isFloating()
    {
        return true;
    }

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
    string getTypeStr() const
    {
        return "PI";
    }

    // Print function
    void printGate() const;  // Self Printing Format
    // void reportFanin(int level) const {};  // None of Fanin
};

class CirPOGate : public CirGate
{
public:
    CirPOGate(const unsigned int &id, const unsigned int &no) : CirGate(id, no) {}
    ~CirPOGate() {}

    // Basic access methods
    string getTypeStr() const
    {
        return "PO";
    }

    // Print function
    void printGate() const;  // Self Printing Format
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
    string getTypeStr() const
    {
        return "AIG";
    }

    // Attribute
    bool isAig() const
    {
        return true;
    }

    // Print function
    void printGate() const;

    // Simulation function
    void operate();
};

class CirGateHash
{
public:
    /**
     * @param[in] c gate which has 2 fanins
     */
    CirGateHash(CirGate *c);
    ~CirGateHash();

    size_t operator()() const;
    bool operator==(const CirGateHash &k) const;

private:
    size_t _in0, _in1;
};

#endif  // CIR_GATE_H
