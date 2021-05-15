/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include "cirGate.h"

#include <stdarg.h>

#include <cassert>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

// Initialize static data member
unsigned int CirGate::_globalMarker = 0;

// Indent
#define INDENT 2

/**************************************/
/*   Public Function for cirGate      */
/**************************************/

bool isInv(size_t c)
{
    return c & size_t(NEG);
}

bool isInv(CirGate *c)
{
    return (size_t)c & size_t(NEG);
}

unsigned int getNum(CirGate *c)
{
    return (unsigned int)(size_t)c;
}

CirGate *setNum(size_t n)
{
    return (CirGate *)n;
}

CirGate *setNum(unsigned int n)
{
    return (CirGate *)(size_t)n;
}

CirGate *setInv(size_t c)
{
    return (CirGate *)(c | size_t(NEG));
}

CirGate *setInv(CirGate *c)
{
    return (CirGate *)((size_t)c | size_t(NEG));
}

CirGate *gate(size_t c)
{
    return (CirGate *)(c & ~size_t(NEG));
}

CirGate *gate(CirGate *c)
{
    return (CirGate *)((size_t)c & ~size_t(NEG));
}

/**************************************/
/*   class CirGate member functions   */
/**************************************/

void CirGate::reportGate() const
{
    vector<vector<CirGate *> > FECGroups;
    vector<vector<CirGate *> >::iterator it;
    vector<CirGate *>::iterator cit;
    stringstream ss;

    // Store informations in stringstream
    ss << "================================================================================"
       << endl;

    // LINE-1: Type and Defined LineNo
    ss << "= " << getTypeStr() << '(' << _gateId << ")";
    if (_symbol != "") {
        ss << '"' << _symbol << '"';
    }
    ss << ", line " << _lineno << endl;

    // LINE-2: FEC-Groups
    ss << "= FECs:";

    // Asked for FECGroups
    FECGroups = cirMgr->getFECGroups();

    for (it = FECGroups.begin(); it != FECGroups.end(); ++it) {
        // Positive
        cit = find(it->begin(), it->end(), this);
        if (cit != it->end()) {
            for (cit = it->begin(); cit != it->end(); ++cit) {
                ss << ' ' << ((isInv(*cit)) ? "!" : "") << gate(*cit)->_gateId;
            }

            break;
        }

        // Negative
        cit = find(it->begin(), it->end(), setInv((CirGate *)this));
        if (cit != it->end()) {
            for (cit = it->begin(); cit != it->end(); ++cit) {
                ss << ' ' << ((isInv(*cit)) ? "" : "!") << gate(*cit)->_gateId;
            }

            break;
        }
    }

    ss << endl;

    // LINE-3: Value
    ss << "= Value: ";
    for (int i = 63; i >= 0; --i) {
        ss << (bool)(((size_t)1 << i) & _state) << ((i % 8 == 0 && i) ? "_" : "");
    }
    ss << endl;

    // Get Lenght of StringStream, fill with space
    // ss.seekp(0, ios::end);
    // if (49 - ss.tellp() > 0) { ss << string(49 - ss.tellp(), ' '); ss << '='
    // << endl; }

    ss << "================================================================================"
       << endl;
    ss.seekp(0, ios::beg);

    // Release string to cout.
    cout << ss.str();
}

void CirGate::reportFanin(int level)
{
    assert(level >= 0);
    raiseGlobalMarker();
    reportFanin(level, 0, false);
}

void CirGate::reportFanout(int level)
{
    assert(level >= 0);
    raiseGlobalMarker();
    reportFanout(level, 0, false);
}

void CirGate::reportFanin(int level, int indent, bool invert)
{
    // Cout state of (this) CirGate, with specified indent
    cout << string(indent, ' ');
    if (invert) {
        cout << '!';
    }
    cout << getTypeStr() << ' ' << _gateId;

    // Cout (*) if the fanin was hidden
    if (isMarked() && _fanin.size() && level) {
        cout << " (*)";
    }

    // End Message;
    cout << endl;

    // For each gate in _fanin should call this function recursively
    if (level > 0 && !isMarked()) {
        mark();

        indent += INDENT;
        for (vector<CirGate *>::iterator it = _fanin.begin(); it != _fanin.end(); ++it) {
            gate(*it)->reportFanin(level - 1, indent, (isInv(*it)) ? true : false);
        }
    }
}

void CirGate::reportFanout(int level, int indent, bool invert)
{
    // Cout state of (this) CirGate, with specified indent
    cout << string(indent, ' ');
    if (invert) {
        cout << '!';
    };
    cout << getTypeStr() << ' ' << _gateId;

    // Cout (*) if the fanout was hidden
    if (isMarked() && _fanout.size() && level) {
        cout << " (*)";
    }

    // End Message;
    cout << endl;

    // For each gate in _fanout should call this function recursively
    if (level > 0 && !isMarked()) {
        mark();

        indent += INDENT;
        for (vector<CirGate *>::iterator it = _fanout.begin(); it != _fanout.end(); ++it) {
            gate(*it)->reportFanout(level - 1, indent, (isInv(*it)) ? true : false);
        }
    }
}

void CirGate::disconnectFanin(CirGate *inGate)
{
    CirGate *gptr;
    int count;

    if (!inGate) {
        for (size_t i = 0; i < _fanin.size(); ++i) {
            gptr = gate(_fanin[i]);
            count = ::count(gptr->_fanout.begin(), gptr->_fanout.end(),
                            (isInv(_fanin[i])) ? setInv(this) : this);
            gptr->_fanout.erase(find(gptr->_fanout.begin(), gptr->_fanout.end(),
                                     (isInv(_fanin[i])) ? setInv(this) : this));
        }
    } else {
        gptr = gate(inGate);
        gptr->_fanout.erase(find(gptr->_fanout.begin(), gptr->_fanout.end(),
                                 (isInv(inGate)) ? setInv(this) : this));
    }
}

void CirGate::disconnectFanout(CirGate *outGate)
{
    CirGate *gptr;

    if (!outGate) {
        for (size_t i = 0; i < _fanout.size(); ++i) {
            gptr = gate(_fanout[i]);
            gptr->_fanin.erase(find(gptr->_fanin.begin(), gptr->_fanin.end(),
                                    (isInv(_fanout[i])) ? setInv(this) : this));
        }
    } else {
        gptr = gate(outGate);
        gptr->_fanin.erase(
            find(gptr->_fanin.begin(), gptr->_fanin.end(), (isInv(outGate)) ? setInv(this) : this));
    }
}

void CirGate::disconnect()
{
    disconnectFanin(NULL);
    disconnectFanout(NULL);
}

void CirGate::addFanin(CirGate *c, bool isInv)
{
    if (isInv) {
        c = setInv(c);
    }
    _fanin.push_back(c);
}

void CirGate::addFanout(CirGate *c, bool isInv)
{
    if (isInv) {
        c = setInv(c);
    }
    _fanout.push_back(c);
}

/***************************************/
/* class CirConstGate member functions */
/***************************************/

void CirConstGate::printGate() const {}

/***************************************/
/* class CirUndefGate member functions */
/***************************************/

void CirUndefGate::printGate() const {}

/***************************************/
/* class CirPIGate member functions    */
/***************************************/

void CirPIGate::printGate() const {}

/***************************************/
/* class CirPOGate member functions    */
/***************************************/

void CirPOGate::printGate() const {}

/***************************************/
/* class CirAIGate member functions    */
/***************************************/

void CirAIGate::printGate() const {}

//------------------------------------------------------------------------
//   class CirGateHash
//------------------------------------------------------------------------

CirGateHash::CirGateHash(CirGate *c) : _in0((size_t)c->_fanin[0]), _in1((size_t)c->_fanin[1]) {}

CirGateHash::~CirGateHash() {}

/**
 * @brief Check whether 2 CirAIGates have the same fanins by hash.
 * @note Self-defined hash function
 * @ref MathStackExchange
 * https://math.stackexchange.com/questions/882877/produce-unique-number-given-two-integers
 */
size_t CirGateHash::operator()() const
{
    return (_in0 + _in1) * (_in0 + _in1 + 1) / 2 + ::min(_in0, _in1);
}

bool CirGateHash::operator==(const CirGateHash &k) const
{
    return ((_in0 == k._in0) && (_in1 == k._in1));
}