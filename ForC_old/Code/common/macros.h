/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Macros
    Main Developer : David Fischer (david.fischer.ch@gmail.com)
    Copyright      : Copyright (c) 2008-2013 smpte2022lib Team. All rights reserved.
    Sponsoring     : Developed for a HES-SO CTI Ra&D project called GaVi
                     Haute école du paysage, d'ingénierie et d'architecture @ Genève
                     Telecommunications Laboratory
\**************************************************************************************************/
/*
  This file is part of smpte2022lib Project.

  This project is free software: you can redistribute it and/or modify it under the terms of the
  EUPL v. 1.1 as provided by the European Commission. This project is distributed in the hope that
  it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE.

  See the European Union Public License for more details.

  You should have received a copy of the EUPL General Public License along with this project.
  If not, see he EUPL licence v1.1 is available in 22 languages:
      22-07-2013, <https://joinup.ec.europa.eu/software/page/eupl/licence-eupl>

  Retrieved from https://github.com/davidfischer-ch/smpte2022lib.git
*/

#ifndef __MACROS__
#define __MACROS__

// Macros ======================================================================

extern void AssertPrintError();

#define POW2(n) (1 << n)
#define RAND(n) ((double)rand()*((double)n/(double)RAND_MAX))

// Macro de la barre d'avancement (pourcentage) ================================

#define PCENT(value,cond100,logFile) \
{ \
  newPcent = 100.0 * (double)(value); \
  if ((newPcent <= 100.0) && (newPcent - oldPcent >= 10.0 || (cond100))) \
  { \
    PRINT0_CON     (cConPcent, "%u%% ", (unsigned)newPcent) \
    PRINT_SET_FILE (logFile, "a") \
    oldPcent = newPcent; \
  } \
}

// Macros de condition de retour ===============================================

#define IFNOT_OP(condition,operation,retour) \
{ \
  if (!(condition)) \
  { \
    operation; \
    return retour; \
  } \
}

#define IFNOT(condition,retour) IFNOT_OP(condition,,retour)

// Macros d'assertion en mode assertions =======================================

#ifdef OPTION_ASSERT_IS_ASSERT

  #define ASSERT_OP(condition,operation,retour,...) \
  { \
    if (!(condition)) \
    { \
      operation; \
      AssertPrintError(); \
      PRINT0_CON  (cConError, "\n") \
      PRINT0_CON  (cConError, __VA_ARGS__) \
      PRINT0_CON  (cConError, " : ") \
      PRINT0_CON  (cConError, #condition) \
      PRINT0_CON  (cConError, "\nLine %u file %s\n", __LINE__, __FILE__) \
      PRINT0_CONc (cConError, cExByeBye) \
      exit (-1); \
    } \
  }

  #define ASSERT_OPc(condition,operation,retour,details) \
    ASSERT_OP(condition,operation,retour,"%s",details)

  #define ASSERT(condition,retour,...) \
    ASSERT_OP(condition,,retour,__VA_ARGS__)

  #define ASSERTc(condition,retour,details) \
    ASSERT_OP(condition,,retour,"%s",details)

#endif

#ifdef OPTION_ASSERTp_IS_ASSERT

  #define ASSERT_OPp(condition,operation,retour,...) \
  { \
    if (!(condition)) \
    { \
      operation; \
      AssertPrintError(); \
      PRINT0_CON  (cConError, "\n") \
      PRINT0_CON  (cConError, __VA_ARGS__) \
      PRINT0_CON  (cConError, " : ") \
      PRINT0_CON  (cConError, #condition) \
      PRINT0_CON  (cConError, "\nLine %u file %s\n", __LINE__, __FILE__) \
      PRINT0_CONc (cConError, cExByeBye) \
      exit (-1); \
    } \
  }

  #define ASSERT_OPpc(condition,operation,retour,details) \
    ASSERT_OPp(condition,operation,retour,"%s",details)

  #define ASSERTp(condition,retour,...) \
    ASSERT_OPp(condition,,retour,__VA_ARGS__)

  #define ASSERTpc(condition,retour,details) \
    ASSERT_OPp(condition,,retour,"%s",details)

#endif

// Macros d'assertion en mode conditions de retour =============================

#ifdef OPTION_ASSERT_IS_IFNOT
  #define ASSERT_OP(condition,operation,retour,...) \
    IFNOT_OP(condition,operation,retour)

  #define ASSERT_OPc(condition,operation,retour,details) \
    IFNOT_OP(condition,operation,retour)

  #define ASSERT(condition,retour,...)      IFNOT(condition,retour)
  #define ASSERTc(condition,retour,details) IFNOT(condition,retour)
#endif

#ifdef OPTION_ASSERTp_IS_IFNOT
  #define ASSERT_OPp(condition,operation,retour,...) \
    IFNOT_OP(condition,operation,retour)

  #define ASSERT_OPpc(condition,operation,retour,details) \
    IFNOT_OP(condition,operation,retour)

  #define ASSERTp(condition,retour,...)      IFNOT(condition,retour)
  #define ASSERTpc(condition,retour,details) IFNOT(condition,retour)
#endif

// Macros d'assertion en mode annhilées ========================================

#ifdef OPTION_ASSERT_IS_NULL
  #define ASSERT_OP(condition,operation,retour,...)
  #define ASSERT_OPc(condition,operation,retour,details)
  #define ASSERT(condition,retour,...)
  #define ASSERTc(condition,retour,details)
#endif

#ifdef OPTION_ASSERTp_IS_NULL
  #define ASSERT_OPp(condition,operation,retour,...)
  #define ASSERT_OPpc(condition,operation,retour,details)
  #define ASSERTp(condition,retour,...)
  #define ASSERTpc(condition,retour,details)
#endif

#endif
