/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include <unordered_map>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

typedef CirGateHash Hash;
typedef std::pair<size_t, CirGate* > HashNode;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

void
CirMgr::strash()
{
   // TODO
   unordered_map<size_t, CirGate*> table;
   unordered_map<size_t, CirGate*>::iterator it;
   vector<CirGate*> dfslist;

   // Make DFSList
   CirGate::raiseGlobalMarker();
   for (size_t i = _M + 1; i < _M + _O + 1; ++i)
      { DepthFirstTraversal(_gates[i], dfslist); }

   // STRASH for each CirAIGate
   for (size_t i = 0; i < dfslist.size(); ++i)
   {
      if (!dfslist[i]->isAig()) { continue; }

      Hash key(dfslist[i]);

      it = table.find(key());

      // If not in hashtable, Insert as a record
      if (it == table.end())
      {
         table.insert(HashNode(key(), dfslist[i]));
      }
      // If some structure already in hash table
      else if (identityStruct(dfslist[i], it->second))
      {
         StrashMsg(dfslist[i], it->second);
         mergeGate(dfslist[i], it->second);
      }
      // If Hash collision only
      else
      {
         table.insert(HashNode(key(), dfslist[i]));
      }
   }

   // Maintain Mgr Property
   _notused.clear();    getNotUsedList(_notused);
   _floating.clear();   getFloatingList(_floating);
}

void
CirMgr::fraig()
{
   if (!FECs.size()) { return; }

   vector<CirGate*> unSATGroup, SATGroup;
   SatSolver solver;
   Var tmpVar;
   bool isSat;
   solver.initialize();

   for (int i = 0; i < _M + 1; ++i)
      { if (_gates[i]) _gates[i]->setVar(solver.newVar()); }

   createCNF(solver);

   sort(FECs.begin(), FECs.end(), [](const vector<CirGate*>& a, const vector<CirGate*>& b){ return a.size() < b.size(); });

   for (size_t i = 0; i < FECs.size(); ++i)
   {
      // Looping elements For each group
      for (size_t j = 0; j < FECs[i].size(); ++j)
      {
         // Init unSAT Group
         unSATGroup.clear();
         unSATGroup.push_back(FECs[i][j]);
         SATGroup.clear();

         // Searching
         for (size_t k = j + 1; k < FECs[i].size(); ++k)
         {
            // Try to prove SAT(f, g)
            tmpVar = solver.newVar();
            solver.addXorCNF(tmpVar, gate(FECs[i][j])->_var, isInv(FECs[i][j]), gate(FECs[i][k])->_var, isInv(FECs[i][k]));
            solver.assumeRelease();
            solver.assumeProperty(tmpVar, true);

            isSat = solver.assumpSolve();

            // getSatAssignment()
            cout << "\rProving " << gate(FECs[i][j])->_gateId << " = "
                  << gate(FECs[i][k])->_gateId << "..."
                  << ((isSat)? "SAT" : "UNSAT\n");

            // Mark if unSAT
            if (!isSat)
               { unSATGroup.push_back(FECs[i][j]); }
         }

         // If unSAT occurs
         if (unSATGroup.size() > 2)
         {
            for (size_t k = 1; k < unSATGroup.size(); ++k)
            {
               FraigMsg(unSATGroup[k], unSATGroup[0]);
               mergeGate(unSATGroup[k], unSATGroup[0]);
               FECs[i].erase(find(FECs[i].begin(), FECs[i].end(), unSATGroup[k]));
            }

            FECs[i].erase(find(FECs[i].begin(), FECs[i].end(), unSATGroup[0]));
         }
      }
   }

   FECs.clear();
}

/********************************************/
/*   Private member functions about strash  */
/********************************************/

/*
   Only Check CirAIGate

   @params a, b
      CirAIGate a, b
*/
bool
CirMgr::identityStruct(CirGate* a, CirGate* b) const
{
   // TODO
   return true;

   if ((a->_fanin.size() != 2) || (b->_fanin.size() != 2))
   { return false; }

   for (size_t i = 0; i < a->_fanin.size(); ++i)
   {
      if (a->_fanin[i] != b->_fanin[i]) { return false; }
   }

   return true;
}

void
CirMgr::StrashMsg(CirGate* from, CirGate* to) const
{
   cout << "Strashing: " << gate(to)->_gateId << " merging " << ((isInv(to))? "!" : "") << from->_gateId << "..." << endl;
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirGate::setVar(const Var& var)
{
   _var = var;
}

Var
CirGate::getVar()
{
   return _var;
}

void
CirMgr::createCNF(SatSolver& solver)
{
   CirGate* target;
   for (int i = 0; i < _aig.size(); ++i)
   {
      target = _gates[_aig[i]];
      solver.addAigCNF(
         target->_var,
         gate(target->_fanin[0])->_var, isInv(target->_fanin[0]),
         gate(target->_fanin[1])->_var, isInv(target->_fanin[1])
      );
   }
}

void
CirMgr::FraigMsg(CirGate* from, CirGate* to) const
{
   cout << "Fraig: " << gate(to)->_gateId << " merging " << ((isInv(to))? "!" : "") << from->_gateId << "..." << endl;
}