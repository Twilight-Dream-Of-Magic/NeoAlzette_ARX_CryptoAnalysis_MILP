#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *#
#*                                                                           *#
#*          This file is part of the program and software framework          *#
#*                    UG --- Ubquity Generator Framework                     *#
#*                                                                           *#
#*  Copyright Written by Yuji Shinano <shinano@zib.de>,                      *#
#*            Copyright (C) 2021-2026 by Zuse Institute Berlin,              *#
#*            licensed under LGPL version 3 or later.                        *#
#*            Commercial licenses are available through <licenses@zib.de>    *#
#*                                                                           *#
#* This code is free software; you can redistribute it and/or                *#
#* modify it under the terms of the GNU Lesser General Public License        *#
#* as published by the Free Software Foundation; either version 3            *#
#* of the License, or (at your option) any later version.                    *#
#*                                                                           *#
#* This program is distributed in the hope that it will be useful,           *#
#* but WITHOUT ANY WARRANTY; without even the implied warranty of            *#
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *#
#* GNU Lesser General Public License for more details.                       *#
#*                                                                           *#
#* You should have received a copy of the GNU Lesser General Public License  *#
#* along with this program.  If not, see <http://www.gnu.org/licenses/>.     *#
#*                                                                           *#
#* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *#

#@file    Makefile
#@brief   Makefile for parallel PBSolver using SCIP as a callable library
#@author  Yuji Shinano

UG_VERSION		:=	1.0.0
#-----------------------------------------------------------------------------
# COMM
#-----------------------------------------------------------------------------
COMM		= cpp11
TSUG            = false
#-----------------------------------------------------------------------------
# paths
#-----------------------------------------------------------------------------
SCIPDIR         =       ../../lib/scip/
UGDIR		=	../..
STPDIR		= 	../../lib/scip/applications/PBSolver
#-----------------------------------------------------------------------------
# include default project Makefile from SCIP
#-----------------------------------------------------------------------------
include $(SCIPDIR)/make/make.project
#-----------------------------------------------------------------------------
# set the UGLIB
#-----------------------------------------------------------------------------
ifeq ($(TSUG), true)
UGLIB		=	tsugscip-$(UG_VERSION).$(BASE).$(COMM)
else
UGLIB		=	ugscip-$(UG_VERSION).$(BASE).$(COMM)
endif # tsug
#-----------------------------------------------------------------------------
# default settings
#-----------------------------------------------------------------------------
TEST            =      short
TIME            =      3600
SETTINGS        =      default
VERBOSE         =      false
OPT             =      opt
COMP            =      gnu
LPS             =      spx
DET             =      false
ZLIB            =      true
THREADS         =      4
#-----------------------------------------------------------------------------
# Main Program
#-----------------------------------------------------------------------------

ifeq ($(COMM), pth)
MAINNAME	=	fscip
endif
ifeq ($(COMM), cpp11)
ifeq ($(TSUG), true)
MAINNAME	=	tsfscip
else
MAINNAME	=	fscip
endif
endif
ifeq ($(COMM), mpi)
ifeq ($(TSUG), true)
MAINNAME	=	tsparascip
else
MAINNAME	=	parascip
endif
endif
ifeq ($(TSUG), true)
UGOBJ		=	pbSolver_ts_plugins.o
else
UGOBJ		=	pbSolver_plugins.o
endif
MAINOBJ         =       message_pb.o \
			event_bestsol.o

UGSRC		=	$(addprefix $(SRCDIR)/,$(UGOBJ:.o=.cpp))
MAINSRC		=	$(addprefix $(STPDIR)/src/,$(MAINOBJ:.o=.c))
MAINSRC         +=      $(addprefix $(SRCDIR)/,$(MAINCXXOBJ:.o=.cpp))

MAIN		=	$(MAINNAME).$(BASE).$(LPS)$(EXEEXTENSION)
MAINFILE	=	$(BINDIR)/$(MAIN)
MAINSHORTLINK	=	$(BINDIR)/$(MAINNAME)
UGOBJFILES	=	$(addprefix $(OBJDIR)/,$(UGOBJ))
MAINOBJFILES	=	$(addprefix $(OBJDIR)/,$(MAINOBJ))
MAINCXXOBJFILES =	$(addprefix $(OBJDIR)/,$(MAINCXXOBJ))

# LASTSETTINGS    =       $(OBJDIR)/make.lastsettings

# maybe setting the include path to SCIP's application directory is enough
CFLAGS		+=	-I$(SCIPDIR)/src -DWITH_UG -I$(STPDIR)/src
ifeq ($(COMM), pth)
CXX             =       g++
LINKCXX         =       g++
CXXFLAGS	+=	-I$(UGDIR)/src -D_COMM_PTH -I$(STPDIR)/src
endif
ifeq ($(COMM), cpp11)
CXX             =       g++
LINKCXX         =       g++
CXXFLAGS	+=	-I$(UGDIR)/src -D_COMM_CPP11 -I$(STPDIR)/src
endif
ifeq ($(COMM), mpi)
CC              =       mpicc
CXX             =       mpicxx
LINKCXX         =       mpicxx
CXXFLAGS	+=	-I$(UGDIR)/src -D_COMM_MPI_WORLD -DMPICH_IGNORE_CXX_SEEK -I$(STPDIR)/src -D_MUTEX_CPP11
endif

LDFLAGS		+=	-lpthread

#-----------------------------------------------------------------------------
# Rules
#-----------------------------------------------------------------------------

ifeq ($(VERBOSE),false)
.SILENT:	$(MAINFILE) $(UGOBJFILES) $(MAINOBJFILES) $(MAINCXXOBJFILES) $(MAINSHORTLINK)
endif

.PHONY: all
all:            $(SCIPDIR) $(MAINFILE) $(MAINSHORTLINK)

.PHONY: lint
lint:		$(MAINSRC)
		-rm -f lint.out
		$(SHELL) -ec 'for i in $^; \
			do \
			echo $$i; \
			$(LINT) $(SCIPDIR)/lint/scip.lnt +os\(lint.out\) -u -zero \
			$(FLAGS) -UNDEBUG -UWITH_READLINE -UROUNDING_FE $$i; \
			done'

.PHONY: scip
scip:
		@$(MAKE) -C $(SCIPDIR) libs $^

$(MAINSHORTLINK):	$(MAINFILE)
		@rm -f $@
		cd $(dir $@) && ln -s $(notdir $(MAINFILE)) $(notdir $@)

$(OBJDIR):
		@-mkdir -p $(OBJDIR)

$(BINDIR):
		@-mkdir -p $(BINDIR)

$(CHECKDIR):
		@-mkdir -p $(CHECKDIR)

.PHONY: test
test:		$(STPDIR)
		@-(ln -fs $(STPDIR)/data);
		@-(cd check && ln -fs ../$(STPDIR)/check/testset);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/configuration_solufile.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/evalcheck.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/evaluate.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/check.awk);

ifeq ($(COMM),pth)
		cd check; \
		$(SHELL) ./check_fscip.sh $(TEST) $(MAINFILE) $(SETTINGS) $(notdir $(MAINFILE)).$(HOSTNAME) $(TIME) $(MEM) $(CONTINUE) $(LOCK) $(LPS) $(THREADS);
endif
ifeq ($(COMM),cpp11)
ifeq ($(TSUG), true)
		cd check; \
		$(SHELL) ./check_tsfscip.sh $(TEST) $(MAINFILE) $(SETTINGS) $(notdir $(MAINFILE)).$(HOSTNAME) $(TIME) $(MEM) $(CONTINUE) $(LOCK) $(LPS) $(THREADS);
else
		cd check; \
		$(SHELL) ./check_fscip.sh $(TEST) $(MAINFILE) $(SETTINGS) $(notdir $(MAINFILE)).$(HOSTNAME) $(TIME) $(MEM) $(CONTINUE) $(LOCK) $(LPS) $(THREADS);
endif
endif
ifeq ($(COMM),mpi)
		cd check; \
		$(SHELL) ./check_parascip.sh $(TEST) $(MAINFILE) $(SETTINGS) $(notdir $(MAINFILE)).$(HOSTNAME) $(TIME) $(MEM) $(CONTINUE) $(LOCK) $(LPS) $(THREADS);
endif

# include local targets
-include $(SCIPDIR)/make/local/make.targets

.PHONY: testclusterf
testclusterf:	$(STPDIR)
		@-(cd check && ln -fs ../$(STPDIR)/check/testset);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/configuration_cluster.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/configuration_logfiles.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/evalcheck_cluster.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/check_cluster.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/configuration_tmpfile_setup_scip.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/allcmpres.sh);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/check.awk);
		@-(cd check && ln -fs ../$(SCIPDIR)/check/cmpres.awk);
		cd check; \
		$(SHELL) ./check_cluster_fscip.sh $(TEST) $(notdir $(MAINFILE)) $(SETTINGS) \
		$(notdir $(MAINFILE)) $(OUTPUTDIR) $(TIME) $(NODES) $(MEM) \
		$(THREADS) $(FEASTOL) $(LPS) $(DISPFREQ) $(CONTINUE) \
		$(QUEUETYPE) $(QUEUE) $(PPN) $(CLIENTTMPDIR) \
		$(NOWAITCLUSTER) $(EXCLUSIVE) $(PERMUTE) $(SEEDS) $(GLBSEEDSHIFT) $(STARTPERM) $(DEBUGTOOL) $(REOPT) $(OPTCOMMAND) $(SETCUTOFF) $(VISUALIZE) $(CLUSTERNODES) $(SLURMACCOUNT) $(PYTHON);

.PHONY: clean
clean:		$(OBJDIR)
ifneq ($(OBJDIR),)
		@-(rm -f $(OBJDIR)/*.o && rmdir $(OBJDIR));
		@echo "-> remove main objective files"
endif
		@-rm -f $(MAINFILE) $(MAINLINK) $(MAINSHORTLINK)
		@echo "-> remove binary"

.PHONY: tags
tags:
		rm -f TAGS; ctags -e src/*.cpp $(SCIPDIR)/src/scip/*.c $(SCIPDIR)/src/scip/*.h;

-include $(LASTSETTINGS)

.PHONY: touchexternal
touchexternal:
		@-rm -f $(LASTSETTINGS)
		@echo >> $(LASTSETTINGS)

-include	$(MAINDEP)

# $(MAINFILE):	$(BINDIR) $(OBJDIR) $(SCIPLIBFILE) $(LPILIBFILE) $(NLPILIBFILE) $(MAINOBJFILES) $(MAINCXXOBJFILES)
$(MAINFILE):	$(BINDIR) $(OBJDIR) $(UGOBJFILES) $(MAINOBJFILES) $(MAINCXXOBJFILES)
		@echo "-> linking $@"
		$(LINKCXX) $(UGOBJFILES) $(MAINOBJFILES) $(MAINCXXOBJFILES) $(LINKCXXSCIPALL) \
		$(LINKCXX_L)$(UGDIR)/lib $(LINKCXX_l)$ $(UGLIB) \
		$(LINKCXXSCIPALL) \
		$(LINKCXX_l)$(LPILIB)$(LINKLIBSUFFIX) \
                $(OFLAGS) $(LPSLDFLAGS) \
		$(LDFLAGS) $(LINKCXX_o)$@

$(OBJDIR)/%.o:	$(STPDIR)/src/%.c
		@echo "-> compiling $@"
		$(CC) $(FLAGS) $(OFLAGS) $(BINOFLAGS) $(CFLAGS) -c $< $(CC_o)$@

$(OBJDIR)/%.o:	$(SRCDIR)/%.cpp
		@echo "-> compiling $@"
		$(CXX) $(FLAGS) $(OFLAGS) $(BINOFLAGS) $(CXXFLAGS) -c $< $(CXX_o)$@

#---- EOF --------------------------------------------------------------------
