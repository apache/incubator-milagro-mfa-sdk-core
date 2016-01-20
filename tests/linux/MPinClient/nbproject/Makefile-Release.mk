#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/_ext/620013993/CvString.o \
	${OBJECTDIR}/_ext/620013993/CvXcode.o \
	${OBJECTDIR}/_ext/580510450/CvHttpRequest.o \
	${OBJECTDIR}/_ext/580510450/CvLogger.o \
	${OBJECTDIR}/_ext/580510450/CvMutex.o \
	${OBJECTDIR}/_ext/580510450/CvSemaphore.o \
	${OBJECTDIR}/_ext/580510450/CvThread.o \
	${OBJECTDIR}/_ext/1341795113/aes.o \
	${OBJECTDIR}/_ext/1341795113/big.o \
	${OBJECTDIR}/_ext/1341795113/ecp.o \
	${OBJECTDIR}/_ext/1341795113/ecp2.o \
	${OBJECTDIR}/_ext/1341795113/ff.o \
	${OBJECTDIR}/_ext/1341795113/fp.o \
	${OBJECTDIR}/_ext/1341795113/fp12.o \
	${OBJECTDIR}/_ext/1341795113/fp2.o \
	${OBJECTDIR}/_ext/1341795113/fp4.o \
	${OBJECTDIR}/_ext/1341795113/gcm.o \
	${OBJECTDIR}/_ext/1341795113/hash.o \
	${OBJECTDIR}/_ext/1341795113/mpin.o \
	${OBJECTDIR}/_ext/1341795113/oct.o \
	${OBJECTDIR}/_ext/1341795113/pair.o \
	${OBJECTDIR}/_ext/1341795113/rand.o \
	${OBJECTDIR}/_ext/1341795113/rom.o \
	${OBJECTDIR}/_ext/1341795113/version.o \
	${OBJECTDIR}/_ext/608447161/mpin_crypto_non_tee.o \
	${OBJECTDIR}/_ext/608447161/mpin_sdk.o \
	${OBJECTDIR}/_ext/608447161/utils.o \
	${OBJECTDIR}/HttpRequest.o \
	${OBJECTDIR}/MpinClient.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=-m64

# CC Compiler Flags
CCFLAGS=-m64
CXXFLAGS=-m64

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-lpthread -lcurl -lcrypto

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mpinclient

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mpinclient: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mpinclient ${OBJECTFILES} ${LDLIBSOPTIONS}

${OBJECTDIR}/_ext/620013993/CvString.o: ../../../ext/cvshared/cpp/CvString.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/620013993
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/620013993/CvString.o ../../../ext/cvshared/cpp/CvString.cpp

${OBJECTDIR}/_ext/620013993/CvXcode.o: ../../../ext/cvshared/cpp/CvXcode.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/620013993
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/620013993/CvXcode.o ../../../ext/cvshared/cpp/CvXcode.cpp

${OBJECTDIR}/_ext/580510450/CvHttpRequest.o: ../../../ext/cvshared/cpp/linux/CvHttpRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/580510450
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/580510450/CvHttpRequest.o ../../../ext/cvshared/cpp/linux/CvHttpRequest.cpp

${OBJECTDIR}/_ext/580510450/CvLogger.o: ../../../ext/cvshared/cpp/linux/CvLogger.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/580510450
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/580510450/CvLogger.o ../../../ext/cvshared/cpp/linux/CvLogger.cpp

${OBJECTDIR}/_ext/580510450/CvMutex.o: ../../../ext/cvshared/cpp/linux/CvMutex.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/580510450
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/580510450/CvMutex.o ../../../ext/cvshared/cpp/linux/CvMutex.cpp

${OBJECTDIR}/_ext/580510450/CvSemaphore.o: ../../../ext/cvshared/cpp/linux/CvSemaphore.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/580510450
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/580510450/CvSemaphore.o ../../../ext/cvshared/cpp/linux/CvSemaphore.cpp

${OBJECTDIR}/_ext/580510450/CvThread.o: ../../../ext/cvshared/cpp/linux/CvThread.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/580510450
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/580510450/CvThread.o ../../../ext/cvshared/cpp/linux/CvThread.cpp

${OBJECTDIR}/_ext/1341795113/aes.o: ../../../src/core/crypto/aes.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/aes.o ../../../src/core/crypto/aes.c

${OBJECTDIR}/_ext/1341795113/big.o: ../../../src/core/crypto/big.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/big.o ../../../src/core/crypto/big.c

${OBJECTDIR}/_ext/1341795113/ecp.o: ../../../src/core/crypto/ecp.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/ecp.o ../../../src/core/crypto/ecp.c

${OBJECTDIR}/_ext/1341795113/ecp2.o: ../../../src/core/crypto/ecp2.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/ecp2.o ../../../src/core/crypto/ecp2.c

${OBJECTDIR}/_ext/1341795113/ff.o: ../../../src/core/crypto/ff.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/ff.o ../../../src/core/crypto/ff.c

${OBJECTDIR}/_ext/1341795113/fp.o: ../../../src/core/crypto/fp.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/fp.o ../../../src/core/crypto/fp.c

${OBJECTDIR}/_ext/1341795113/fp12.o: ../../../src/core/crypto/fp12.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/fp12.o ../../../src/core/crypto/fp12.c

${OBJECTDIR}/_ext/1341795113/fp2.o: ../../../src/core/crypto/fp2.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/fp2.o ../../../src/core/crypto/fp2.c

${OBJECTDIR}/_ext/1341795113/fp4.o: ../../../src/core/crypto/fp4.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/fp4.o ../../../src/core/crypto/fp4.c

${OBJECTDIR}/_ext/1341795113/gcm.o: ../../../src/core/crypto/gcm.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/gcm.o ../../../src/core/crypto/gcm.c

${OBJECTDIR}/_ext/1341795113/hash.o: ../../../src/core/crypto/hash.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/hash.o ../../../src/core/crypto/hash.c

${OBJECTDIR}/_ext/1341795113/mpin.o: ../../../src/core/crypto/mpin.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/mpin.o ../../../src/core/crypto/mpin.c

${OBJECTDIR}/_ext/1341795113/oct.o: ../../../src/core/crypto/oct.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/oct.o ../../../src/core/crypto/oct.c

${OBJECTDIR}/_ext/1341795113/pair.o: ../../../src/core/crypto/pair.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/pair.o ../../../src/core/crypto/pair.c

${OBJECTDIR}/_ext/1341795113/rand.o: ../../../src/core/crypto/rand.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/rand.o ../../../src/core/crypto/rand.c

${OBJECTDIR}/_ext/1341795113/rom.o: ../../../src/core/crypto/rom.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/rom.o ../../../src/core/crypto/rom.c

${OBJECTDIR}/_ext/1341795113/version.o: ../../../src/core/crypto/version.c 
	${MKDIR} -p ${OBJECTDIR}/_ext/1341795113
	${RM} "$@.d"
	$(COMPILE.c) -O2 -I../../../src/core/crypto -I../../../src/core -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/1341795113/version.o ../../../src/core/crypto/version.c

${OBJECTDIR}/_ext/608447161/mpin_crypto_non_tee.o: ../../../src/core/mpin_crypto_non_tee.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/608447161
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/608447161/mpin_crypto_non_tee.o ../../../src/core/mpin_crypto_non_tee.cpp

${OBJECTDIR}/_ext/608447161/mpin_sdk.o: ../../../src/core/mpin_sdk.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/608447161
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/608447161/mpin_sdk.o ../../../src/core/mpin_sdk.cpp

${OBJECTDIR}/_ext/608447161/utils.o: ../../../src/core/utils.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/608447161
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/_ext/608447161/utils.o ../../../src/core/utils.cpp

${OBJECTDIR}/HttpRequest.o: HttpRequest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HttpRequest.o HttpRequest.cpp

${OBJECTDIR}/MpinClient.o: MpinClient.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MpinClient.o MpinClient.cpp

${OBJECTDIR}/main.o: main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -I../../../src/core/crypto -I../../../src/core -I../../../ext/cvshared/cpp/include -I../../../src/core/json -I../../../src/core/utf8 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mpinclient

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
