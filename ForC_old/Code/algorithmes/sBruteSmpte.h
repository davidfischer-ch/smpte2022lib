/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Brute force implementation of SMPTE 2022-1 FEC
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

#ifndef __SBRUTESMPTE__
#define __SBRUTESMPTE__

// Types de données ============================================================

// Structure stockant ce qu'il faut pour l'algorithme de SMPTE 2022-1 brute ------
typedef struct
{
  sBufferMedia media; //. Stockage des paquets de média
  sLinkedList  fec;   //. Stockage des paquets de FEC

  bool overwriteMedia; //. Ecrasage des doublons dans buffer média autorisé ?

  unsigned recovered;            //. Nombre de paquets média récupérés
  unsigned unrecoveredOnReading; //. Nb paq. média manquants lors de la lecture !

  unsigned nbArPaMedia; //. Nombre d'appels à ArriveePaquetMedia
  unsigned nbArPaFec;   //. Nombre d'appels à ArriveePaquetFec
  unsigned nbLePaMedia; //. Nombre d'appels à LecturePaquetMedia
  unsigned nbApFec;     //. Nombre d'appels à AppliqueFec

  unsigned maxF; //. Nombre max d'éléments stockés dans FEC

  clock_t chronoTotal; //. Temps d'exécution total en TICKS
  clock_t chronoMedia; //. Temps d'exécution total pour média en TICKS
  clock_t chronoFec;   //. Temps d'exécution total pour FEC   en TICKS
}
  sBruteSmpte;

// Déclaration des fonctions ===================================================

sBruteSmpte sBruteSmpte_New (bool pOverwriteMedia);

void sBruteSmpte_Release (sBruteSmpte*);
void sBruteSmpte_Print   (const sBruteSmpte*, bool pBuffers);

void sBruteSmpte_ArriveePaquetMedia (sBruteSmpte*, sPaquetMedia*);
void sBruteSmpte_ArriveePaquetFec   (sBruteSmpte*, sPaquetFec*);
bool sBruteSmpte_LecturePaquetMedia (sBruteSmpte*, sMediaNo pBufferSize, FILE*);

void sBruteSmpte_AppliqueFec (sBruteSmpte*);

#endif
