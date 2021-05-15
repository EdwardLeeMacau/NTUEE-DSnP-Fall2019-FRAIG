/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/

// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   // TODO
   stack<CirGate*> frontier;
   CirGate* target = 0;
   CirGate* inGate = 0;
   size_t i = 0;
   bool invert = false;

   // Iterately remove and update _notused
   for (size_t i = 0; i < _notused.size(); ++i)
   {
      target = _gates[_notused[i]];

      for (size_t j = 0; j < target->_fanin.size(); ++j)
      {
         inGate = gate(target->_fanin[j]);
         invert = isInv(target->_fanin[j]);

         target->disconnectFanin(target->_fanin[j]);

         // If inGate has no fanout, Mark as NOTUSED GATE
         if (!inGate->_fanout.size() && !dynamic_cast<CirConstGate* >(inGate) )
            { _notused.push_back(inGate->_gateId); }
      }
   }

   // Maintain CirMgr Message.
   sort(_notused.begin(), _notused.end());

   i = 0;
   while (i < _notused.size())
   {
      target = _gates[_notused[i]];

      if (target->isAig() || target->isFloating())
      {
         cout << "Sweeping: " << target->getTypeStr() << '(' << target->_gateId << ") removed..." << endl;
         removeGate(target);
         _notused.erase(_notused.begin() + i);
      }
      else { ++i; }
   }

   i = 0;
   while (i < _floating.size())
   {
      if (!_gates[_floating[i]])
         { _floating.erase(_floating.begin() + i); }
      else
         { ++i; }
   }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
   // TODO

   vector<CirGate* > dfslist;
   CirGate* target;
   int num;
   bool phase;

   // Build up DFS List
   CirGate::raiseGlobalMarker();
   for (size_t o = 0; o < _O; ++o)
      { DepthFirstTraversal(_M + o + 1, dfslist); }

   // Iterate DFS List, and check if CASE 1~4 satisfy
   for (size_t i = 0; i < dfslist.size(); ++i)
   {
      target = dfslist[i];

      if (hasConstFanin(target, num) && target->isAig())
      {
         // Case 1: CONST 0 in CirAIGate->_fanin[num]
         if (!isInv(target->_fanin[num]))
         {
            SimplifyMsg(target, target->_fanin[num]);
            mergeGate(target, target->_fanin[num]);
         }

         // Case 2: CONST 1 in CirAIGate->_fanin[num]
         else if (isInv(target->_fanin[num]))
         {
            SimplifyMsg(target, target->_fanin[1 - num]);
            mergeGate(target, target->_fanin[1 - num]);
         }
      }
      else if (identityFanin(target, phase))
      {
         // Case 3: Invert Fanin
         if (phase)
         {
            SimplifyMsg(target, _gates[0]);
            mergeGate(target, _gates[0]);
         }
         // Case 4: Identical Fanin
         else
         {
            SimplifyMsg(target, target->_fanin[0]);
            mergeGate(target, target->_fanin[0]);
         }
      }
   }

   // Maintain cirMgr Property
   _notused.clear();    getNotUsedList(_notused);
   _floating.clear();   getFloatingList(_floating);
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

bool
CirMgr::removeGate(const unsigned int& id)
{
   return removeGate(_gates[id]);
}

/*
   (API) Type of removing any Gate
*/
bool
CirMgr::removeGate(CirGate* gate)
{
   if (dynamic_cast<CirAIGate* >(gate))
      { return removeAIGate(gate); }
   if (dynamic_cast<CirUndefGate* >(gate))
      { return removeUndefGate(gate); }
   // Actually Only AIGate and UndefGate will be removed
   if (dynamic_cast<CirPIGate* >(gate))
      { return removePIGate(gate); }
   if (dynamic_cast<CirPOGate* >(gate))
      { return removePOGate(gate); }

   return false;
}

bool
CirMgr::removePIGate(CirGate* gate)
{
   return false;
}

bool
CirMgr::removePOGate(CirGate* gate)
{
   return false;
}

/*

*/
bool
CirMgr::removeAIGate(CirGate* gate)
{
   unsigned int id = gate->_gateId;

   // Release the Memory
   delete _gates[id]; _gates[id] = 0;

   // Maintain Mgr Attribute
   _aig.erase(find(_aig.begin(), _aig.end(), id));
   --_A;

   return true;
}

/*

*/
bool
CirMgr::removeUndefGate(CirGate* gate)
{
   unsigned int id = gate->_gateId;

   // Release the Memory
   delete _gates[id]; _gates[id] = 0;

   // Maintain Mgr Attribute
   /* Nothing */

   return true;
}

/*
   @param from, to
      The gate to be merged or ready to merge.
*/
void
CirMgr::mergeGate(CirGate* from, CirGate* to)
{
   vector<CirGate*>::iterator it;
   bool setInvert;

   for (size_t i = 0; i < from->_fanout.size(); ++i)
   {
      setInvert = isInv(to) ^ isInv(from->_fanout[i]);

      // Copy the link to merge gate.
      gate(to)->addFanout(gate(from->_fanout[i]), setInvert);

      // Update the link for FanOuts
      *find(
         gate(from->_fanout[i])->_fanin.begin(),
         gate(from->_fanout[i])->_fanin.end(),
         isInv(from->_fanout[i])? setInv(from) : from
      ) = (setInvert)? setInv(to) : gate(to);
   }

   // from->_fanout.clear();
   from->disconnectFanin(NULL);
   removeGate(from);
   return;
}

/*
   InfoMsg for CirMgr::optimize()
*/
void
CirMgr::SimplifyMsg(CirGate* from, CirGate* to) const
{
   cout << "Simplifying: " << gate(to)->_gateId << " merging " << ((isInv(to))? "!" : "") << from->_gateId << "..." << endl;
}

/*
   HasConstFanin is consider as true when only c is
   type CirAIGate or CirPOGate.

   @param num
      Index of the CONST fanin
*/
bool
CirMgr::hasConstFanin(CirGate* c, int& num) const
{
   for (size_t i = 0; i < c->_fanin.size(); ++i)
   {
      if (gate(c->_fanin[i])->isConst())
         { num = i; return true; }
   }

   return false;
}

/*
   identityFanin is consider as true when only c is
   type CirAIGate.

   The parameter phase is set as true if the fanins
   is inverter identical.
*/
bool
CirMgr::identityFanin(CirGate* c, bool& phase) const
{
   if (c->_fanin.size() != 2) return 0;

   size_t i = ((size_t)(c->_fanin[0]) ^ (size_t)(c->_fanin[1]));

   if (i < 2) { phase = (bool)i; return true; }

   return false;
}