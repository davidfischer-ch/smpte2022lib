/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Console utilities
    Authors     : David Fischer
    Contact     : david.fischer.ch@gmail.com / david.fischer@hesge.ch
    Copyright   : 2008-2013 smpte2022lib Team. All rights reserved.
    Sponsoring  : Developed for a HES-SO CTI Ra&D project called GaVi
                  Haute école du paysage, d'ingénierie et d'architecture @ Genève
                  Telecommunications Laboratory
\**************************************************************************************************/
/*
  This file is part of smpte2022lib.

  This project is free software: you can redistribute it and/or modify it under the terms of the
  GNU General Public License as published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along with this project.
  If not, see <http://www.gnu.org/licenses/>

  Retrieved from:
    git clone git://github.com/davidfischer-ch/smpte2022lib.git
*/

#ifndef __CONSOLE__
#define __CONSOLE__

// Macros de sélection du mode console ou fichier ==============================

#ifdef OPTION_OS_IS_WINDOWS
  #define DEFAULT_FOR_CONSOLE "CON"
#else
  #define DEFAULT_FOR_CONSOLE "/dev/tty"
#endif

#define PRINT_SET_CON() \
{ \
  if (!freopen (DEFAULT_FOR_CONSOLE, "w", stdout)) \
  { \
    printf ("%s", cExFreopen); \
    printf ("%s", cExByeBye); \
    exit (-1); \
  } \
}

#define PRINT_SET_FILE(destination,mode) \
{ \
  if (!freopen (destination, mode, stdout)) \
  { \
    printf ("%s", cExFreopen); \
    printf ("%s", cExByeBye); \
    exit (-1); \
  } \
}

// Macros de sélection de la couleur d'écriture (console) ======================

#ifdef OPTION_OS_IS_WINDOWS
  #define PRINT_INIT_COLOR() hConsole = GetStdHandle (STD_OUTPUT_HANDLE);
  #define PRINT_SET_COLOR(color) SetConsoleTextAttribute (hConsole, color);
#else
  #define PRINT_INIT_COLOR() {}
  #define PRINT_SET_COLOR(color) printf("%s", color);
#endif

// Macros d'écriture dirigé obligatoirement dans la console ====================

#define PRINT0_CON(color,...) \
{ \
  PRINT_SET_CON   () \
  PRINT_SET_COLOR (color) \
  PRINT0          (__VA_ARGS__) \
  PRINT_SET_COLOR (cConDefault) \
}

#define PRINT0_CONc(color,details) \
{ \
  PRINT_SET_CON   () \
  PRINT_SET_COLOR (color) \
  PRINT0c         (details) \
  PRINT_SET_COLOR (cConDefault) \
}

// Macros d'écriture dirigé obligatoirement dans un fichier ====================

#define PRINT0_FILE(destination,mode,...) \
{ \
  PRINT_SET_FILE (destination, mode) \
  PRINT0         (__VA_ARGS__) \
}

#define PRINT0_FILEc(destination,mode,details) \
{ \
  PRINT_SET_FILE (destination, mode) \
  PRINT0c        (details) \
}

// Macros d'écriture de niveau 1 (principal) ===================================

#define PRINT0(...) printf (__VA_ARGS__);
#define PRINT0c(details) printf ("%s", details);

// Macros d'écriture de niveau 1 (secondaire) ==================================

#ifdef OPTION_PRINT1_IS_NULL
  #define PRINT1(...) {}
  #define PRINT1c(details) {}
#else
  #define PRINT1(...) if (verbose >= 1) { printf (__VA_ARGS__); }
  #define PRINT1c(details) if (verbose >= 1) { printf ("%s", details); }
#endif

// Macros d'écriture de niveau 2 (tertiaire) ===================================

#ifdef OPTION_PRINT2_IS_NULL
  #define PRINT2(...)
  #define PRINT2c(details)
  #define DETAILS2(...)
#else
  #define PRINT2(...) if (verbose >= 2) { PRINT1(__VA_ARGS__) }
  #define PRINT2c(...) if (verbose >= 2) { PRINT1(details) }
  #define DETAILS2(...) if (verbose >= 2) { __VA_ARGS__ }
#endif

// Constantes publiques ========================================================

#ifdef OPTION_OS_IS_WINDOWS

  extern const unsigned cConBlack;        //. Code couleur pour la console
  extern const unsigned cConBlue;         //. Code couleur pour la console
  extern const unsigned cConGreen;        //. Code couleur pour la console
  extern const unsigned cConCyan;         //. Code couleur pour la console
  extern const unsigned cConRed;          //. Code couleur pour la console
  extern const unsigned cConMagenta;      //. Code couleur pour la console
  extern const unsigned cConBrown;        //. Code couleur pour la console
  extern const unsigned cConLightGrey;    //. Code couleur pour la console
  extern const unsigned cConDarkGrey;     //. Code couleur pour la console
  extern const unsigned cConLightBlue;    //. Code couleur pour la console
  extern const unsigned cConLightGreen;   //. Code couleur pour la console
  extern const unsigned cConLightCyan;    //. Code couleur pour la console
  extern const unsigned cConLightRed;     //. Code couleur pour la console
  extern const unsigned cConLightMagenta; //. Code couleur pour la console
  extern const unsigned cConYellow;       //. Code couleur pour la console
  extern const unsigned cConWhite;        //. Code couleur pour la console
  extern const unsigned cConBlink;        //. Code couleur pour la console

  extern const unsigned cConDefault; //. Code couleur pour la console
  extern const unsigned cConError;   //. Code couleur pour la console
  extern const unsigned cConWarning; //. Code couleur pour la console
  extern const unsigned cConPcent;   //. Code couleur pour la console
  extern const unsigned cConTitle;   //. Code couleur pour la console

#else

  extern const char* cConBlack;        //. Code couleur pour la console
  extern const char* cConBlue;         //. Code couleur pour la console
  extern const char* cConGreen;        //. Code couleur pour la console
  extern const char* cConCyan;         //. Code couleur pour la console
  extern const char* cConRed;          //. Code couleur pour la console
  extern const char* cConMagenta;      //. Code couleur pour la console
  extern const char* cConBrown;        //. Code couleur pour la console
  extern const char* cConLightGrey;    //. Code couleur pour la console
  extern const char* cConDarkGrey;     //. Code couleur pour la console
  extern const char* cConLightBlue;    //. Code couleur pour la console
  extern const char* cConLightGreen;   //. Code couleur pour la console
  extern const char* cConLightCyan;    //. Code couleur pour la console
  extern const char* cConLightRed;     //. Code couleur pour la console
  extern const char* cConLightMagenta; //. Code couleur pour la console
  extern const char* cConYellow;       //. Code couleur pour la console
  extern const char* cConWhite;        //. Code couleur pour la console
  extern const char* cConBlink;        //. Code couleur pour la console

  extern const char* cConDefault; //. Code couleur pour la console
  extern const char* cConError;   //. Code couleur pour la console
  extern const char* cConWarning; //. Code couleur pour la console
  extern const char* cConPcent;   //. Code couleur pour la console
  extern const char* cConTitle;   //. Code couleur pour la console

#endif

// Variables publiques =========================================================

#ifdef OPTION_OS_IS_WINDOWS
  extern HANDLE hConsole; //. Handle utilisé pour la colorisation de la console
#endif

extern unsigned verbose; //. Faut-il être verbeux ?

// Fonctions publiques =========================================================

char  KeyToContinue (bool pAutoKey);
char* UtilePourMoi  (const char*, const char*);
char* GetParameterValue (const char* pArg, const char* pLabel, char pSeparator);


#endif
