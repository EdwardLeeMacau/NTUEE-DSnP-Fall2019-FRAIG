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
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

#include "cirDef.h"
#include "rnGen.h"

// TODO: Feel free to define your own classes, variables, or functions.

extern CirMgr *cirMgr;

class CirMgr
{
public:
    CirMgr() {}
    ~CirMgr() {}

    // Access functions
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate *getGate(unsigned gid) const;

    // Member functions about circuit construction
    bool readCircuit(const std::string &);

    // Member functions about circuit optimization
    /**
     * @brief Remove unused gates
     * @details UNDEF, float and unused list may be changed. DFS list should NOT
     * be changed.
     */
    void sweep();
    /**
     * @brief Recursively simplifying from POs;
     * _dfsList needs to be reconstructed afterwards
     * UNDEF gates may be delete if its fanout becomes empty...
     */
    void optimize();

    // Member functions about simulation
    /**
     * @brief Run simulation in random to generate FEC groups
     */
    void randomSim();
    /**
     * @brief Run simulation in preset input patterns to generate FEC groups
     */
    void fileSim(std::ifstream &);
    void setSimLog(std::ofstream *logFile)
    {
        _simLog = logFile;
    }

    // Member functions about fraig
    void strash();
    void printFEC() const;
    void fraig();

    // Member functions about circuit reporting
    /**
     * @brief Print number of gates in circuit
     */
    void printSummary() const;
    /**
     * @brief Forward (from PIn to POut) printing
     */
    void printNetlist() const;
    /**
     * @brief Print all PIn gates
     * @see CirMgr::printPOs()
     * @see CirMgr::printFloatGates()
     */
    void printPIs() const;
    /**
     * @brief Print all POut gates
     * @see CirMgr::printPIs()
     * @see CirMgr::printFloatGates()
     */
    void printPOs() const;
    /**
     * @brief Print all floating gates
     * @details Floating gates: floating fanins, or gates that are defined but
     * not used
     * @see CirMgr::printPIs()
     * @see CirMgr::printPOs()
     */
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(std::ostream &) const;
    void writeGate(std::ostream &, CirGate *) const;

    // Get FECGroups
    const std::vector<std::vector<CirGate *> > &getFECGroups() const;

    // Member functions about cirMgr
    void reset();

private:
    std::ofstream *_simLog;

    // DFSList Maintainer or builder
    void DepthFirstTraversal(const unsigned int, std::vector<CirGate *> &) const;
    void DepthFirstTraversal(CirGate *, std::vector<CirGate *> &) const;

    // Loader Function
    bool readHeader(const std::string &);
    bool loadInput(const unsigned int &);
    /**
     * @brief Output pin will have no fanout, but 1 fanin
     * @param[in] id    The ID Number of the Gate
     * @param[in] fanin The ID Number of the Fanin
     * @return bool True if successfully load a POut
     */
    bool loadOutput(const unsigned int &, const unsigned int &);
    /**
     * @brief And-Inverter Gate will have 2 fanin, variant fanout.
     * @param id The ID Number of the Gate Output
     * @param i1, i2 The ID Number of the fanin (input)
     */
    bool loadAIG(const unsigned int &);
    /**
     * @brief Load the latches
     * @note Not supported yet.
     */
    bool loadLatch(const unsigned int &, const unsigned int &);
    bool loadSymbol(const unsigned int &, const std::string &);
    bool loadComment(const std::string &);

    // Mgr Basic Attribute
    void getFloatingList(std::vector<unsigned int> & /* floating */);
    void getNotUsedList(std::vector<unsigned int> & /* notused */);

    // Gate Property Modifier
    void connect(CirGate *, CirGate *);
    void sortInOut();
    void sortIn();
    void sortOut();

    // Gate Property Modifier (Defined outside cirMgr.cpp)
    bool removeGate(const unsigned int &);
    bool removeGate(CirGate *);
    bool removePIGate(CirGate *);
    bool removePOGate(CirGate *);
    bool removeAIGate(CirGate *);
    bool removeUndefGate(CirGate *);
    void replaceConnection(CirGate *, CirGate *, CirGate *);
    void mergeGate(CirGate *, CirGate *);

    // Gate Property Identifier (Defined outside cirMgr.cpp)
    /**
     * @details identityFanin() is consider as true when only c is type
     * CirAIGate. The parameter phase is set as true if the fanin is inverter
     * identical.
     */
    bool identityFanin(CirGate *, bool & /* phase */) const;
    /**
     * @details HasConstFanin() is consider as true when only c is type
     * CirAIGate or CirPOGate.
     * @param[in]  c   pointer to AI Gate or PO Gate
     * @param[out] num Index of the CONST fanin
     */
    bool hasConstFanin(CirGate *, int & /* index */) const;
    /**
     * @brief Check AI gates a and b are identical
     * @param[in] a, b CirAIGate a, b
     */
    bool identityStruct(CirGate *, CirGate *) const;

    // FRAIG Function
    void createCNF(SatSolver & /* solver */);

    // Simulation Pattern
    void encodePattern(const std::vector<size_t> & /* txPatterns */,
                       const std::vector<size_t> & /* rxPatterns */, int /* maskLength */,
                       std::ostream * /* os */) const;
    int parsePattern(std::vector<std::vector<size_t> > & /* patternsGroups */,
                     std::istream & /* is */) const;
    int genPattern(std::vector<size_t> & /* txPatterns */) const;

    // Simulation Function
    void splitFECGroups(int /* maskLength */, bool /* again */);
    void initFECGroups();
    void simulateOnce(const std::vector<size_t> & /* txPatterns */, int /* maskLength */,
                      bool /* again */);
    void feedSignal(const std::vector<size_t> & /* txPatterns */);
    void getSignal(std::vector<size_t> & /* rxPatterns */);

    // Message Printer
    void SimplifyMsg(CirGate *, CirGate *) const;
    void StrashMsg(CirGate *, CirGate *) const;
    void FraigMsg(CirGate *, CirGate *) const;

    std::vector<unsigned int> _pin;       // PinIn Number
    std::vector<unsigned int> _pout;      // PinOut Number
    std::vector<unsigned int> _aig;       // AIGs Number
    std::vector<unsigned int> _latch;     // Latch Number
    std::vector<unsigned int> _floating;  // Floating Gates
    std::vector<unsigned int> _notused;   // Not in used Gates
    std::vector<CirGate *> _gates;

    std::stringstream _comment;  // Store the comment

    unsigned int _M;  // Maximal Variable Index
    unsigned int _I;  // Number of Inputs
    unsigned int _L;  // Number of Latches
    unsigned int _O;  // Number of Outputs
    unsigned int _A;  // Number of AND Gates

    std::vector<std::vector<CirGate *> > FECs;  // Functionally Equivalent Candidate (FEC) Groups
    RandomNumGen rnGen;                         // Random Number Generator
};

#endif  // CIR_MGR_H
