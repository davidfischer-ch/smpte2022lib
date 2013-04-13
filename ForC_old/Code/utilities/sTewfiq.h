/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : TODO
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

#ifndef __STEWFIQ__
#define __STEWFIQ__

// Types de données ============================================================

// Structure représentant un Tewfiq --------------------------------------------

typedef struct
{
  bool     etat;           //. Etat (ok ou perte) en cours
  uint32_t nombreOk;       //. Statistiques (nombre de non perte)
  uint32_t nombrePertes;   //. Statistiques (nombre de pertes)
  uint32_t quantitePertes; //. Statistiques (nombre de passages à)
  double   optionP;        //. Probabilité de passer de ok à perte sur 1.0
  double   optionQ;        //. Probabilité de passer de perte à ok sur 1.0
} sTewfiq;

// Déclaration des Fonctions ===================================================

float sTewfiq_DistribUni();
float sTewfiq_DistribExp();
float sTewfiq_DistribPareto();
float sTewfiq_DistribGauss (float pVariance);

sTewfiq sTewfiq_New1 ();
sTewfiq sTewfiq_New2 (double pOptionP, double pOptionQ);

bool sTewfiq_IsOkayOrLost (      sTewfiq*);
void sTewfiq_Print        (const sTewfiq*);

#endif
