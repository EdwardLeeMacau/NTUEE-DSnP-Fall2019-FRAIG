/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
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

bool
isInv(size_t c)
{
   return c & size_t(NEG);
}

bool
isInv(CirGate* c)
{
   return (size_t)c & size_t(NEG);
}

unsigned int
getNum(CirGate* c)
{
   return (unsigned int)(size_t)c;
}

CirGate*
setNum(size_t n)
{
   return (CirGate*)n;
}

CirGate*
setNum(unsigned int n)
{
   return (CirGate*)(size_t)n;
}

CirGate*
setInv(size_t c)
{
   return (CirGate*)(c | size_t(NEG));
}

CirGate*
setInv(CirGate* c)
{
   return (CirGate*)((size_t)c | size_t(NEG));
}

CirGate*
gate(size_t c)
{
   return (CirGate*)(c & ~size_t(NEG));
}

CirGate*
gate(CirGate* c)
{
   return (CirGate*)((size_t)c &~size_t(NEG));
}

/**************************************/
/*   class CirGate member functions   */
/**************************************/

void
CirGate::reportGate() const
{
   vector<vector<CirGate*>> FECGroups;
   vector<vector<CirGate*>>::iterator it;
   vector<CirGate*>::iterator cit;
   stringstream ss;

   // Store informations in stringstream
   ss << "================================================================================" << endl;

   // LINE-1: Type and Defined LineNo
   ss << "= " << getTypeStr() << '(' << _gateId << ")";
   if (_symbol != "") { ss << '"' << _symbol << '"'; }
   ss << ", line " << _lineno << endl;

   // LINE-2: FEC-Groups
   ss << "= FECs:";

   // Asked for FECGroups
   FECGroups = cirMgr->getFECGroups();

   for (it = FECGroups.begin(); it != FECGroups.end(); ++it )
   {
      // Positive
      cit = find(it->begin(), it->end(), this);

      if (cit != it->end())
      {
         for (cit = it->begin(); cit != it->end(); ++cit)
         {
            ss << ' ' << ((isInv(*cit))? "!" : "") << gate(*cit)->_gateId;
         }

         break;
      }

      // Negative
      cit = find(it->begin(), it->end(), setInv((CirGate*)this));

      if (cit != it->end())
      {
         for (cit = it->begin(); cit != it->end(); ++cit)
         {
            ss << ' ' << ((isInv(*cit))? "" : "!") << gate(*cit)->_gateId;
         }

         break;
      }
   }

   ss << endl;


   // LINE-3: Value
   ss << "= Value: ";
   for (int i = 63; i >= 0; --i)
      { ss << (bool)(((size_t)1 << i) & _state) << ((i % 8 == 0 && i)? "_" : ""); }
   ss << endl;

   // Get Lenght of StringStream, fill with space
   // ss.seekp(0, ios::end);
   // if (49 - ss.tellp() > 0) { ss << string(49 - ss.tellp(), ' '); ss << '=' << endl; }

   ss << "================================================================================" << endl;
   ss.seekp(0, ios::beg);

   // Release string to cout.
   cout << ss.str();
}

// Public Function (API)
void
CirGate::reportFanin(int level)
{
   assert (level >= 0);
   raiseGlobalMarker();
   reportFanin(level, 0, false);
}

// Public Function (API)
void
CirGate::reportFanout(int level)
{
   assert (level >= 0);
   raiseGlobalMarker();
   reportFanout(level, 0, false);
}

/***************************************/
/* class CirGate protect functions     */
/***************************************/

// (Private) Report Fanin
void
CirGate::reportFanin(int level, int indent, bool invert)
{
   // Cout state of (this) CirGate, with specified indent
   cout << string(indent, ' ');
   if (invert) { cout << '!'; }
   cout << getTypeStr() << ' ' << _gateId;

   // Cout (*) if the fanin was hidden
   if (isMarked() && _fanin.size() && level)
      { cout << " (*)"; }

   // End Message;
   cout << endl;

   // Recursive Call _fanin to Report
   if (level > 0 && !isMarked())
   {
      // Mark (this) CirGate
      mark();

      // Call reportFanin for each Fanin(s)
      indent += INDENT;
      for (vector<CirGate*>::iterator it = _fanin.begin(); it != _fanin.end(); ++it)
         gate(*it)->reportFanin(level - 1, indent, (isInv(*it))? true : false);
   }
}

// (Private) Report Fanout
void
CirGate::reportFanout(int level, int indent, bool invert)
{
   // Cout state of (this) CirGate, with specified indent
   cout << string(indent, ' ');
   if (invert) cout << '!';
   cout << getTypeStr() << ' ' << _gateId;

   // Cout (*) if the fanout was hidden
   if (isMarked() && _fanout.size() && level) cout << " (*)";

   // End Message;
   cout << endl;

   // Recursive Call _fanin to Report
   if (level > 0 && !isMarked())
   {
      // Mark (this) CirGate
      mark();

      // Call reportFanouts for each Fanout(s)
      indent += INDENT;
      for (vector<CirGate*>::iterator it = _fanout.begin(); it != _fanout.end(); ++it)
         gate(*it)->reportFanout(level - 1, indent, (isInv(*it))? true : false);
   }
}

/*
   @param inGate
      Disconnect 1 if specified, disconnect all if NULL.
*/
void
CirGate::disconnectFanin(CirGate* inGate)
{
   CirGate* gptr;
   int count;

   if (!inGate)
   {
      for (size_t i = 0; i < _fanin.size(); ++i)
      {
         gptr = gate(_fanin[i]);
         count = ::count(gptr->_fanout.begin(), gptr->_fanout.end(), (isInv(_fanin[i]))? setInv(this) : this);
         gptr->_fanout.erase(
            find(
               gptr->_fanout.begin(),
               gptr->_fanout.end(),
               (isInv(_fanin[i]))? setInv(this) : this
            )
         );
      }
   }
   else
   {
      gptr = gate(inGate);
      gptr->_fanout.erase(
         find(
            gptr->_fanout.begin(),
            gptr->_fanout.end(),
            (isInv(inGate))? setInv(this) : this
         )
      );
   }
}

/*
   @param inGate
      Disconnect 1 if specified, disconnect all if NULL.
*/
void
CirGate::disconnectFanout(CirGate* outGate)
{
   CirGate* gptr;

   if (!outGate)
   {
      for (size_t i = 0; i < _fanout.size(); ++i)
      {
         gptr = gate(_fanout[i]);
         gptr->_fanin.erase(
            find(gptr->_fanin.begin(), gptr->_fanin.end(), (isInv(_fanout[i]))? setInv(this) : this)
         );
      }
   }
   else
   {
      gptr = gate(outGate);
      gptr->_fanin.erase(
         find(gptr->_fanin.begin(), gptr->_fanin.end(), (isInv(outGate))? setInv(this) : this)
      );
   }
}

void
CirGate::disconnect()
{
   disconnectFanin(NULL);
   disconnectFanout(NULL);
}

/*
   Append CirGate* to _fanin

   @param c
      Fanin in type CirGate*
   @param isInv
      If True, store 0x1 at the LSB of CirGate*
*/
void
CirGate::addFanin(CirGate* c, bool isInv)
{
   if (isInv) c = setInv(c);
   _fanin.push_back(c);
}

/*
   Append CirGate* to _fanout

   @param c
      Fanout in type CirGate*
   @param isInv
      If True, store 0x1 at the LSB of CirGate*
*/
void
CirGate::addFanout(CirGate* c, bool isInv)
{
   if (isInv) c = setInv(c);
   _fanout.push_back(c);
}

/***************************************/
/* class CirConstGate member functions */
/***************************************/

void
CirConstGate::printGate() const
{

}

/***************************************/
/* class CirUndefGate member functions */
/***************************************/

void
CirUndefGate::printGate() const
{

}

/***************************************/
/* class CirPIGate member functions    */
/***************************************/

void
CirPIGate::printGate() const
{

}


/***************************************/
/* class CirPOGate member functions    */
/***************************************/

void
CirPOGate::printGate() const
{

}


/***************************************/
/* class CirAIGate member functions    */
/***************************************/

void
CirAIGate::printGate() const
{

}
