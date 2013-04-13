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

#include "../smpte.h"

// Constantes privées ==========================================================

const sTewfiq INIT_TEWFIQ = //. Valeur par défaut d'un Tewfiq
  {true, 0, 0, 0, 0.001, 1.000};

// Fonctions publiques =========================================================

// Distribution Uniforme [0,1] -------------------------------------------------
//> Valeur calculée via la loi de probabilité
float sTewfiq_DistribUni()
{
  return (float)rand() / (float)RAND_MAX;
}

// Distribution Exponentielle f(x) = e^-x --------------------------------------
//> Valeur calculée via la loi de probabilité
float sTewfiq_DistribExp()
{
  return - log (1 - sTewfiq_DistribUni() / 1.0001);
}

// Distribution de Pareto f(x) = a/b(b/x)^(a+1) avec a=2 b=1 -------------------
//> Valeur calculée via la loi de probabilité
float sTewfiq_DistribPareto()
{
  return 1.0 / sqrt (1.0001 - sTewfiq_DistribUni());
}

// Distribution Gaussienne (enfin ça a la forme...)                       ------
// Remarque : Trouvé sur http://rfv.insa-lyon.fr/~jolion/STAT/node56.html ------
//> Valeur calculée via la loi de probabilité
float sTewfiq_DistribGauss (float pVariance)
{
	float tMax = 100.0, tEui = 0.0;
	int i; for (i = 0; i < tMax; i++) { tEui += sTewfiq_DistribUni(); }
	return (sqrt (12.0 / tMax) * (tEui - tMax / 2.0)) * pVariance;
}

// Création d'un nouveau tewfiq ------------------------------------------------
//> Nouveau tewfiq
sTewfiq sTewfiq_New1()
{
  //srand((unsigned)time(NULL));

  return INIT_TEWFIQ;
}

// Création d'un nouveau tewfiq ------------------------------------------------
//> Nouveau tewfiq
sTewfiq sTewfiq_New2
  (double pOptionP, //: Valeur désirée pour la probabilité P
   double pOptionQ) //: Valeur désirée pour la probabilité Q
{
  //srand((unsigned)time(NULL));

  sTewfiq _tewfiq = INIT_TEWFIQ;
  _tewfiq.optionP = pOptionP;
  _tewfiq.optionQ = pOptionQ;
  return _tewfiq;
}

//              (1-p)
//              ------
//             |      |
//             |      v
//     ---- (état ok media) <----
//    |                         |
//  p |                         | q
//    |                         |
//     --> (état perte media) ---
//             ^      |
//             |      |
//              ------
//              (1-q)

// Simule un médium réseau (avec les probabilités de passage à perte / à ok) ---
//> Etat (true = ok, false = perte) de paquet
bool sTewfiq_IsOkayOrLost
  (sTewfiq* pTewfiq) //: Tewfiq utilisé
{
  ASSERTpc (pTewfiq, false, cExNullPtr)

  if (pTewfiq->etat)
  {
    if (RAND(1000) <= pTewfiq->optionP * 1000)
    {
      pTewfiq->etat = false;
      pTewfiq->quantitePertes++;
    }
  }
  else
  {
    if (RAND(1000) <= pTewfiq->optionQ * 1000) pTewfiq->etat = true;
  }

  if (pTewfiq->etat) pTewfiq->nombreOk    ++;
  else               pTewfiq->nombrePertes++;

  return pTewfiq->etat;
}

// Affiche (et enregistre dans un fichier) les statistiques Tewfiq :-)     -----
// (Qui est le nom de mon ex professeur de télétrafique, alias télétewfiq) -----
void sTewfiq_Print
  (const sTewfiq* pTewfiq) //: Tewfiq utilisé
{
  ASSERTpc (pTewfiq,, cExNullPtr)

  double _PourcentOk =
    (100.0 * pTewfiq->nombreOk) / (pTewfiq->nombreOk + pTewfiq->nombrePertes);

  PRINT1 ("statistiques avant traitement de FEC \n"
          "nombre de paquets media arrives sans embuche = %lu (%g %%)\n"
          "nombre de paquets media perdus (simule) = %lu (%g %%)\n"
          "longueur moyenne d'un burst de perte = %g\n\n",
          pTewfiq->nombreOk,             _PourcentOk,
          pTewfiq->nombrePertes, 100.0 - _PourcentOk,
          (double)pTewfiq->nombrePertes / (double)pTewfiq->quantitePertes)
}
