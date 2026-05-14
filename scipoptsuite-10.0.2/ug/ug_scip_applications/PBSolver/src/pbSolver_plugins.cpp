/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                           */
/*          This file is part of the program and software framework          */
/*                    UG --- Ubquity Generator Framework                     */
/*                                                                           */
/*  Copyright Written by Yuji Shinano <shinano@zib.de>,                      */
/*            Copyright (C) 2021-2026 by Zuse Institute Berlin,              */
/*            licensed under LGPL version 3 or later.                        */
/*            Commercial licenses are available through <licenses@zib.de>    */
/*                                                                           */
/* This code is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU Lesser General Public License        */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU Lesser General Public License for more details.                       */
/*                                                                           */
/* You should have received a copy of the GNU Lesser General Public License  */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>.     */
/*                                                                           */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/**@file   pbSolver_plugins.cpp
 * @brief  Pseudo Boolean Solver user plugins
 * @author Yuji Shinano
 */

/*--+----1----+----2----+----3----+----4----+----5----+----6----+----7----+----8----+----9----+----0----+----1----+----2*/

#include "ug_scip/scipUserPlugins.h"
#include "ug_scip/scipParaSolver.h"
#include "ug_scip/scipParaInitiator.h"

#include "scip/scipdefplugins.h"
#include "message_pb.h"
#include "event_bestsol.h"

using namespace UG;
using namespace ParaSCIP;

/* to provide rank ans size to user  */
static ParaComm *paraComm = 0;

class PBSolverUserPlugins : public ScipUserPlugins {
public:
   SCIP_MESSAGEHDLR* messagehdlr = nullptr;

   void operator()(SCIP* scip, const char* logname = nullptr, bool quiet = false, bool comment = false)
   {
      if( comment )
      {
         /* create own buffered message handler for PB output and overrides default scip message handler */
         SCIP_CALL_ABORT( SCIPcreateMessagehdlrPbSolver(&messagehdlr, TRUE, logname, quiet) );

         /* if we are running the PB solver, we use this message handler to produce the required output */
         SCIP_CALL_ABORT( SCIPsetMessagehdlr(scip, messagehdlr) );
      }
   }

   void writeUserSolution(SCIP* scip, int nSolvers, double dual)
   {
      if( messagehdlr != nullptr )
         SCIPprintSolutionPbSolver(scip);
      else
         SCIPprintSolutionStatistics(scip, NULL);
   }

//   void newSubproblem(
//         SCIP *scip,
//         const ScipParaDiffSubproblemBranchLinearCons *linearConss,
//         const ScipParaDiffSubproblemBranchSetppcCons *setppcConss)
//   {
//      if( linearConss && setppcConss )
//      {
//         initReceivedSubproblem(scip, linearConss->nLinearConss, linearConss->consNames, setppcConss->nSetppcConss, setppcConss->consNames);
//      } else if( linearConss && !setppcConss )
//      {
//         initReceivedSubproblem(scip, linearConss->nLinearConss, linearConss->consNames, 0, 0);
//      } else if( !linearConss && setppcConss )
//      {
//         initReceivedSubproblem(scip, 0, 0, setppcConss->nSetppcConss, setppcConss->consNames);
//      } else
//      {
//         initReceivedSubproblem(scip, 0, 0, 0, 0);
//      }
//
//   }

};


void
setUserPlugins(ParaInitiator *inInitiator)
{
   ScipParaInitiator *initiator = dynamic_cast<ScipParaInitiator *>(inInitiator);
   initiator->setUserPlugins(new PBSolverUserPlugins());
   paraComm = initiator->getParaComm();
}

void
setUserPlugins(ParaInstance *inInstance)
{
   ScipParaInstance *instance = dynamic_cast<ScipParaInstance *>(inInstance);
   instance->setUserPlugins(new PBSolverUserPlugins());
}

void
setUserPlugins(ParaSolver *inSolver)
{
   ScipParaSolver *solver = dynamic_cast<ScipParaSolver *>(inSolver);
   solver->setUserPlugins(new PBSolverUserPlugins());
   if( !paraComm )
   {
      paraComm = solver->getParaComm();
   }
}

//extern "C"
//int getUgRank()
//{
//   return paraComm->getRank();
//}
//
//extern "C"
//int getUgSize()
//{
//   return paraComm->getSize();
//}
//
//extern "C"
//const char*
//getBranchLinearConsName(const char* names, int i)
//{
//   const char *name = names;
//   for( int j = 0; j < i; j++)
//   {
//      assert(*name);
//      name += (std::strlen(name) + 1);
//   }
//   assert(*name);
//   return name;
//}
//
//extern "C"
//const char*
//getBranchSetppcConsName(const char* names, int i)
//{
//   const char *name = names;
//   for( int j = 0; j < i; j++)
//   {
//      assert(*name);
//      name += (std::strlen(name) + 1);
//   }
//   assert(*name);
//   return name;
//}




