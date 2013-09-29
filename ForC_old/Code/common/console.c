/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Console utilities
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

#include "../smpte.h"

// Constantes couleurs utilisés ================================================

#ifdef OPTION_OS_IS_WINDOWS

  const unsigned cConBlack        =   0;
  const unsigned cConBlue         =   1;
  const unsigned cConGreen        =   2;
  const unsigned cConCyan         =   3;
  const unsigned cConRed          =   4;
  const unsigned cConMagenta      =   5;
  const unsigned cConBrown        =   6;
  const unsigned cConLightGrey    =   7;
  const unsigned cConDarkGrey     =   8;
  const unsigned cConLightBlue    =   9;
  const unsigned cConLightGreen   =  10;
  const unsigned cConLightCyan    =  11;
  const unsigned cConLightRed     =  12;
  const unsigned cConLightMagenta =  13;
  const unsigned cConYellow       =  14;
  const unsigned cConWhite        =  15;
  const unsigned cConBlink        = 128;

  const unsigned cConDefault = 15;
  const unsigned cConError   = 12;
  const unsigned cConWarning = 14;
  const unsigned cConPcent   = 11;
  const unsigned cConTitle   =  9;

#else

  const char* cConBlack        = "\033[22;30m";
  const char* cConRed          = "\033[22;31m";
  const char* cConGreen        = "\033[22;32m";
  const char* cConBrown        = "\033[22;33m";
  const char* cConBlue         = "\033[22;34m";
  const char* cConMagenta      = "\033[22;35m";
  const char* cConCyan         = "\033[22;36m";
  const char* cConGray         = "\033[22;37m";
  const char* cConDarkGrey     = "\033[01;30m";
  const char* cConLightRed     = "\033[01;31m";
  const char* cConLightGreen   = "\033[01;32m";
  const char* cConYellow       = "\033[01;33m";
  const char* cConLightBlue    = "\033[01;34m";
  const char* cConLightMagenta = "\033[01;35m";
  const char* cConLightCyan    = "\033[01;36m";
  const char* cConWhite        = "\033[01;37m";

  const char* cConDefault = "\033[22;30m";
  const char* cConError   = "\033[22;31m";
  const char* cConWarning = "\033[01;33m";
  const char* cConPcent   = "\033[22;36m";
  const char* cConTitle   = "\033[22;34m";

#endif

// Variables publiques =========================================================

#ifdef OPTION_OS_IS_WINDOWS
  HANDLE hConsole = NULL;
#endif

unsigned verbose = 0;

// Fonctions publiques =========================================================

// Attend la frappe d'une touche par l'utilisateur -----------------------------
//> Caractère correspondant à la touche frappée
char KeyToContinue
  (bool pAutoKey) //: Validation inutile ?
{
  if (pAutoKey) return 0;

  printf ("\nPress a key to continue ...\n");
  fflush (stdout);
  return getchar();
}

// Petites manipulations de nom de fichier -------------------------------------
//> Chaîne représentant source + suffixe . extension
char* UtilePourMoi
  (const char* pSource,  //: Chaîne source (nom de fichier)
   const char* pSuffixe) //: Suffixe à ajouter entre la fin du nom et l'ext.
{
  ASSERTpc (pSource,  0, cExNullPtr)
  ASSERTpc (pSuffixe, 0, cExNullPtr)

  char*  _extension       = strrchr (pSource, '.');
  size_t _lengthSource    = strlen  (pSource);
  size_t _lengthSuffixe   = strlen  (pSuffixe);
  size_t _lengthExtension = _extension ? strlen (_extension) : 0;
  char*  _destination     = malloc (_lengthSource + _lengthSuffixe + 1);
  IFNOT (_destination, 0)

  strcpy (_destination, pSource);

  _destination[_lengthSource - _lengthExtension] = 0;

  strcat (_destination, pSuffixe);

  if (_extension)
  {
    strcat (_destination, _extension);
  }

  return _destination;
}

// TODO ---
//> TODO
char* GetParameterValue (const char* pArg, const char* pLabel, char pSeparator)
{
  size_t size = strlen (pLabel);

  // Si le paramètre n'est pas suffisamment long
  if (strlen (pArg) < size + 2) return 0;

  // Si le label du paramètre ne correspond pas
  if (memcmp (pArg, pLabel, size) != 0) return 0;

  // Si le paramètre ne contient pas le bon séparateur
  if (pArg[size] != pSeparator) return 0;

  // Retourne la valeur du paramètre
  return (char*)&pArg[size + 1];
}
