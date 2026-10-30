# 2021-04-27 오후 4:13:10 hkkim:  comment out  NFS_HOME LOCAL_HOME

#
# Project dependent make variables
#
# PRODUCT(S)  : KR SDP(Smart Digital Processor) Development
#				ACE-SDP-ESIO
#
# MODIFICATION LOG :
#   Date      Who    Rev                       Comments
# ---------- ------  ----  -------------------------------------------
# 2011.11.07 ChoiBC   01   create
# 2015.12.10 ChoiBC   02   copy from pmu
# 2016.05.27 ChoiBC   03   final
# 2016.07.11 ChoiBC   04   PPC_SANE 추가

#
#	TARGET_PLATFORM : LINUX, PPC_SAEIL, PPC_SANE
#
#TARGET_PLATFORM = LINUX
#TARGET_PLATFORM = PPC_SAEIL
TARGET_PLATFORM = PPC_SANE


#	-D_INTEL_TARGET : for linux
ifeq ($(TARGET_PLATFORM), LINUX)
PROJ_CFLAGS = -D_INTEL_TARGET -D_PLATFORM_LINUX
else
PROJ_CFLAGS = -D_PLATFORM_PPC
endif

#	-D_SDP : SDP Project, if not defined then Editor or Monitor 
PROJ_CFLAGS += -D_SDP 

#
# Environment
#
ifeq ($(TARGET_PLATFORM), PPC_SANE)
PLATFORM      = PPC
#NFS_HOME      = /home/freescale/esio_home
#LOCAL_HOME    = /media/sf_shared-SDP/ACE-SDP-ESIO-V3
endif
ifeq ($(TARGET_PLATFORM), PPC_SAEIL)
PLATFORM      = PPC
NFS_HOME      =  /home/freescale/esio_home
LOCAL_HOME    = /media/sf_Shared/ACE-SDP-ESIO-V3
endif
ifeq ($(TARGET_PLATFORM), LINUX)
PLATFORM      = LINUX
NFS_HOME      = /home/freescale/bin
LOCAL_HOME    = /media/sf_Shared/ACE-SDP-ESIO-V3
endif





#
# Variables
#
MAKE        = make
CD          = cd
RM          = rm
MV          = mv
CP          = cp
#CC          = $(CROSS_COMPILE)gcc
#LD          = $(CROSS_COMPILE)ld
#AR          = $(CROSS_COMPILE)ar
ARFLAG 	    = rcv
#RANLIB      = $(CROSS_COMPILE)ranlib
#STRIP       = $(CROSS_COMPILE)strip
#OBJCOPY     = $(CROSS_COMPILE)objcopy
#OBJDUMP     = $(CROSS_COMPILE)objdump
CTAGS       = ctags
FIND        = find

#
# User
#
USER_INC = $(LOCAL_HOME)/inc
USER_LIB = $(LOCAL_HOME)/lib
USER_BIN = $(LOCAL_HOME)/bin

INCLUDEDIRS += -I. -I$(USER_INC)

#
# Static or Shared Library
#
#LIB_EXTENSION = so
LIB_EXTENSION = a

#
# libLocal
#
LIBLOCALNAME = libLocal

# files that deleted by make clean
TEMP_FILES = *.bak tags core

#
# Application
#
#FLAGS_APP = $(PROJ_CFLAGS) -O2 -D_REENTRANT -Wall -Werror -D_CHECK_LOGS
FLAGS_APP = -g -D_REENTRANT -Wall $(PROJ_CFLAGS)

#
# 61850 Library
#
# DFLAG
# 		_l  : logging    / no debug
# 		_n  : no logging / no debug
# 		_ld : logging    / debug
# 		_nd : no logging / debug
#
ifeq ($(PLATFORM), PPC)
DFLAG=_l
DEFS=-DDEBUG_SISCO
else
DFLAG=_ld
DEFS=-DDEBUG_SISCO
endif

#
# IEC61850
#
MMSLITEDIR = ../mmslite
MMSLITEINCDIR = $(MMSLITEDIR)/inc
MMSLITELIBDIR = ../mmslite.lib

MMSLITELIBS = $(MMSLITELIBDIR)/mvlu$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mlogl$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mmsle$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mmsl$(DFLAG).a\
			  			$(MMSLITELIBDIR)/asn1l$(DFLAG).a\
			  			$(MMSLITELIBDIR)/mem$(DFLAG).a\
			  			$(MMSLITELIBDIR)/slog$(DFLAG).a\
			  			$(MMSLITELIBDIR)/util$(DFLAG).a \
			  			$(MMSLITELIBDIR)/ositcps$(DFLAG).a

LIBS_MMSLITE  = -lpthread -lrt

#FLAGS_MMSLITE = -fsigned-char -D_GNU_SOURCE -D_THREAD_SAFE -pthread -m32
# 이 건 mmsliste에만 적용되나 보다.
#FLAGS_MMSLITE  = -D_GNU_SOURCE -D_THREAD_SAFE -pthread -m32
#  hkkim -m32지우고
FLAGS_MMSLITE  = -D_GNU_SOURCE -D_THREAD_SAFE -pthread 
FLAGS_MMSLITE += $(DEFS)
FLAGS_MMSLITE += -DMMS_LITE -DMOSI -DLEAN_T -DMLOG_ENABLE -DUSE_DIB_LIST -DMVL_UCA 

#
# 61850 lib
#
DIRlib61850 = $(LOCAL_HOME)/61850Lib
LIBNAME61850  = lib61850

#
# Mini-Xml Library
#
DIRmxml = $(LOCAL_HOME)/mxml
ifeq ($(PLATFORM), PPC)
MXMLLIBS = $(MMSLITELIBDIR)/libmxml.PPC.a
else
MXMLLIBS = $(MMSLITELIBDIR)/libmxml.a
endif

# END
