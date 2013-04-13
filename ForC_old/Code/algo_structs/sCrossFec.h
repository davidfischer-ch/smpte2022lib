/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Link between a media packet <-> FEC packets able to recover it
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

#ifndef __SCROSSFEC__
#define __SCROSSFEC__

// Types de données ============================================================

// Structure liant médiaNo et les paquets de FEC colonne + ligne liés ----------
typedef struct
{
  sFecNx colNx; //. fecNo du paquet de FEC colonne "lié" au médiaNo spécifique
  sFecNx rowNx; //. fecNo du paquet de FEC ligne   "lié" au médiaNo spécifique
}
  sCrossFec;

// Déclaration des Fonctions ===================================================

sCrossFec* sCrossFec_New     ();
void       sCrossFec_Release (      sCrossFec*);
void       sCrossFec_Print   (const sCrossFec*);
#endif
