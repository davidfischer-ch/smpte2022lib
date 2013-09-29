/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Null-able media/FEC packet sequence number
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

// Constantes publiques ========================================================

const sMediaNx MEDIA_NX_NULL={0, true}; //. Valeur par défaut de médiaNx (~null)
const sFecNx   FEC_NX_NULL  ={0, true}; //. Valeur par défaut de fecNx (~null)

// Fonctions publiques =========================================================

// Retourne un médiaNx à partir d'un médiaNo -----------------------------------
//> Le numéro de séquence médiaNx équivalent
inline sMediaNx sMediaNo_to_sMediaNx
  (sMediaNo pMediaNo) //: Le médiaNo à convertir
{
  return (sMediaNx) {pMediaNo, false};
}

// Retourne un fecNx à partir d'un fecNo ---------------------------------------
//> Le numéro de séquence fecNx équivalent
inline sFecNx sFecNo_to_sFecNx
  (sFecNo pFecNo) //: Le fecNo à convertir
{
  return (sFecNx) {pFecNo, false};
}

// Affiche le contenu d'un médiaNx ---------------------------------------------
inline void sMediaNx_Print
  (sMediaNx pMediaNx) //: Le médiaNx à afficher
{
  if (pMediaNx.null) { PRINT1 ("mediaNx {null} ")           }
  else               { PRINT1 ("mediaNx {%u} ", pMediaNx.v) }
}

// Affiche le contenu d'un fecNx -----------------------------------------------
inline void sFecNx_Print
  (sFecNx pFecNx) //: Le fecNx à afficher
{
  if (pFecNx.null) { PRINT1 ("fecNx {null} ")         }
  else             { PRINT1 ("fecNx {%u} ", pFecNx.v) }
}
