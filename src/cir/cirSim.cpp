/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <unordered_map>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

typedef vector<CirGate*> FECGroup;
typedef vector<FECGroup> FECGroups;
typedef pair<size_t, FECGroup> FECHashNode;
typedef unordered_map<size_t, FECGroup> FECHash;

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public functions about Simulation          */
/************************************************/

/*
   Util Function for inverting size_t
*/
size_t
invert(const size_t& num)
{
   return SIZE_T_MAX - num;
}

/*
   Generate ALL 1 mash
*/
size_t
mask(int length)
{
   return ((size_t)1 << (length - 1)) | (((size_t)1 << (length - 1)) - 1);
}

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/

void
CirMgr::randomSim()
{
   // TODO
   vector<size_t> txPattern(_I, 0);
   unsigned int count = 0;
   unsigned int prevFECs;
   bool criteria = true;

   // Init FECGroups with CirGate
   initFECGroups();

   while (criteria)
   {
      // Record #FEC
      prevFECs = FECs.size();

      // Simulate with rnGen
      count += genPattern(txPattern);
      simulateOnce(txPattern, 64, (count != 64));

      cout << "\rTotal #FEC Group = " << FECs.size();
      fflush(NULL);

      criteria = (prevFECs != FECs.size()) && (FECs.size()) ;
   }

   // Sort FEC Group
   sort(FECs.begin(), FECs.end(),
      [](vector<CirGate*> a, vector<CirGate*> b)
         { return gate(a[0])->_gateId < gate(b[0])->_gateId; }
   );

   // Final Result
   cout << '\r' << count << " patterns simulated." << endl;
   return;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
   // TOOO
   vector<vector<size_t>> txPatterns;
   unsigned int count = 0;

   // Parse the Pattern File
   count = parsePattern(txPatterns, patternFile);

   if (count)
   {
      initFECGroups();

      // Simulate with File
      for (size_t i = 0; i < txPatterns.size(); ++i)
      {
         simulateOnce(txPatterns[i], ((count - i * 64) > 64)? 64 : count - i * 64, (i != 0));

         cout << "\rTotal #FEC Group = " << FECs.size();
         fflush(NULL);
      }
   }

   // Sort FEC Group
   sort(FECs.begin(), FECs.end(),
      [](vector<CirGate*> a, vector<CirGate*> b)
         { return gate(a[0])->_gateId < gate(b[0])->_gateId; }
   );

   // Final Result
   cout << '\r' << count << " patterns simulated." << endl;
   return;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

/*
   Algorithm: All-Gate Simulation

   @params txPatterns
      The patterns array for all input
*/
void
CirMgr::simulateOnce(const vector<size_t>& txPatterns, int maskLength, bool again)
{
   vector<size_t> rxPatterns(_A, 0);

   feedSignal(txPatterns);
   getSignal(rxPatterns);
   splitFECGroups(maskLength, again);

   if (_simLog)
      encodePattern(txPatterns, rxPatterns, maskLength, _simLog);
}

/*
   Give Input Signal to AIG.
*/
void
CirMgr::feedSignal(const vector<size_t>& patterns)
{
   // For each CirPIGate, set the value
   for (size_t i = 0; i < _I; ++i)
      { _gates[_pin[i]]->setState(patterns[i]); }
}

/*
   Get output signal of AIG.
*/
void
CirMgr::getSignal(vector<size_t>& patterns)
{
   vector<CirGate*> dfslist;

   // Build up dfslist
   CirGate::raiseGlobalMarker();
   for (size_t o = _M + 1; o < _M + _O + 1; ++o)
      { DepthFirstTraversal(_gates[o], dfslist); }

   // For each gate, get the value
   for (size_t i = 0; i < dfslist.size(); ++i)
      { dfslist[i]->operate(); }

   // For each gate, get the value
   for (size_t o = 0; o < _O; ++o)
      { patterns[o] = _gates[_pout[o]]->getState() ; }
}

/*
   Handling 2 cases
   1. Just initialize the FEC Groups
   2. Running over some patterns
*/
void
CirMgr::splitFECGroups(int maskLength, bool again)
{
   // TODO: Fixed Complement Output Problem
   FECGroups newFECs;
   FECHash hashTable;
   FECHash::iterator hashIt;
   size_t value;
   size_t m = mask(maskLength);
   bool setInvert;

   for (size_t i = 0; i < FECs.size(); ++i)
   {
      // Init: Clean the hashTable for each FEC Group
      hashTable.clear();

      // hashTable only maintain 1 new FEC Group for 1 simulation value
      for (FECGroup::iterator it = FECs[i].begin(); it != FECs[i].end(); ++it)
      {
         // Handling Complement Bits Problems
         value = gate(*it)->getState();

         // If keep running the Simulations...
         if (again)
            { setInvert = isInv(*it); }
         // If it's first time to run FEC Simulations...
         else if (value > (m >> 1))
            { setInvert = true; }
         else
            { setInvert = false; }


         if (setInvert) { value = invert(value) & m; }

         // Hashing
         hashIt = hashTable.find(value);

         // Find exists group
         if (hashIt != hashTable.end())
            { hashIt->second.push_back((setInvert)? setInv(*it) : *it); }
         // No exists group here, initialize one
         else
            { hashTable.insert( FECHashNode(value, vector<CirGate*>(1, (setInvert)? setInv(*it) : *it)) ); }
      }

      // Add to new FEC Groups when #gates in group > 1
      for (FECHash::iterator it = hashTable.begin(); it != hashTable.end(); ++it)
      {
         if ((it->second).size() > 1)
            { newFECs.push_back(it->second); }
      }
   }

   // Finally replace the FEC Groups information
   FECs = newFECs;
}

void
CirMgr::initFECGroups()
{
   // Init FECGroups with CirGate
   // if (FECs.empty())
   // {
   FECs.clear();
   FECs.push_back(FECGroup());
   FECs[0].push_back(_gates[0]);
   for (size_t i = 0; i < _pin.size(); ++i)
      { if (_gates[_pin[i]]) { FECs[0].push_back(_gates[_pin[i]]); } }
   for (size_t a = 0; a < _aig.size(); ++a)
      { if (_gates[_aig[a]]) { FECs[0].push_back(_gates[_aig[a]]); } }
   // for (size_t o = 0; o < _pout.size(); ++o)
   //    { if (_gates[_pout[o]]) { FECs[0].push_back(_gates[_pout[o]]); } }
   // }
}

int
CirMgr::parsePattern(vector<vector<size_t>>& patterns, istream& is) const
{
   vector<size_t> ptn(_I, 0);
   string ptnStr = "";
   int count = 0, tmp;

   while (getline(is, ptnStr))
   {
      ptnStr.erase(remove_if(ptnStr.begin(), ptnStr.end(), ::isspace), ptnStr.end());

      // Error Detection
      if (!ptnStr.length())
      {
         /* ignore this line */
         continue;
      }
      else if (ptnStr.length() < _I)
      {
         cout << "Bad Pattern: The length of pattern: " << ptnStr.length()
              << ", excepted PI: " << _I << endl;

         return count;
      }
      else if (ptnStr.length() > _I)
      {
         cout << endl << "Error: Pattern(" << ptnStr << ") length("
              << ptnStr.length() << ") does not match the number of inputs("
              << _I << ") in a circuit!!" << endl;

         return count;
      }

      // Feed the bits into patterns
      for (int i = 0; i < _I; ++i)
      {
         tmp = (int)ptnStr[i] - 48;

         // Error Detection
         if (tmp != 0 && tmp != 1)
         {
            cout << endl << "Error: Pattern(" << ptnStr
                 << ") contains a non-0/1 character('"
                 << ptnStr[i] << "')." << endl;

            return false;
         }

         ptn[i] = (ptn[i] << 1) + tmp;
      }

      // To count a line was prased successfully.
      ++count;

      if (count % 64 == 0)
         { patterns.push_back(ptn); fill(ptn.begin(), ptn.end(), 0); }
   }

   patterns.push_back(ptn);
   return count;
}

void
CirMgr::encodePattern(const vector<size_t>& txPattern, const vector<size_t>& rxPattern, int maskLength, ostream* os) const
{
   // size_t parallelMask = mask(maskLength);
   size_t bitMask = (size_t)1 << (maskLength - 1);

   for (int i = 0; i < maskLength; ++i)
   {
      for (int j = 0; j < _I; ++j)
         { *(os) << ((bool)(txPattern[j] & bitMask)); }
      *(os) << ' ';
      for (int j = 0; j < _O; ++j)
         { *(os) << ((bool)(rxPattern[j] & bitMask)); }
      *(os) << endl;

      bitMask = bitMask >> 1;
   }
}

/*
   Generate 64 bits Pattern
*/
int
CirMgr::genPattern(vector<size_t>& patterns) const
{
   for (vector<size_t>::iterator it = patterns.begin(); it != patterns.end(); ++it)
      { *it = ((size_t)rnGen(INT_MAX) << 32) + rnGen(INT_MAX); }

   return 64;
}

/*************************************************/
/*   Private member functions about CirGate      */
/*************************************************/

const size_t&
CirGate::getState()
{
   return _state;
}

void
CirGate::setState(const size_t& s)
{
   _state = s;
}

void
CirGate::operate()
{
   /* Do Nothing */
}

/*************************************************/
/*  Private member functions about CirConstGate  */
/*************************************************/

void
CirConstGate::operate()
{
   /* Do Nothing */
}

const size_t&
CirConstGate::getState()
{
   return 0;
}

/*************************************************/
/*  Private member functions about CirUndefGate  */
/*************************************************/

/*
void
CirUndefGate::operate()
{
   // throw "Not vaild for CirUndefGate::operate() (" + to_string(_gateId) + ")\n";
}

const size_t&
CirUndefGate::getState()
{
   // throw "Not vaild for CirUndefGate::getState() (" + to_string(_gateId) + ")\n";
   return 0;
}
*/

/*************************************************/
/*  Private member functions about CirPOGate     */
/*************************************************/

/*
   For CirPOGate, State = Fanin.State
*/
void
CirPOGate::operate()
{
   _state = isInv(_fanin[0])? invert(gate(_fanin[0])->getState()) : gate(_fanin[0])->getState();
}

/*************************************************/
/*  Private member functions about CirAIGate     */
/*************************************************/

/*
   For CirAIGate, State = A & B
*/
void
CirAIGate::operate()
{
   size_t in[2];

   in[0] = isInv(_fanin[0])? invert(gate(_fanin[0])->getState()) : gate(_fanin[0])->getState();
   in[1] = isInv(_fanin[1])? invert(gate(_fanin[1])->getState()) : gate(_fanin[1])->getState();

   _state = in[0] & in[1];
}
