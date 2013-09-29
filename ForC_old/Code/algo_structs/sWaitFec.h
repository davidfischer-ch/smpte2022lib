/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : FEC packet stored for future usage
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

#ifndef __SWAITFEC__
#define __SWAITFEC__

// Types de données ============================================================

// Structure stockant un paquet de FEC en attente d'être utilisé ---------------
typedef struct
{
  sFecNo     fecNo;           //. FecNo du paquet de FEC
  uint16_t   Length_recovery; //. Longueur du payload (pResXor)
  uint8_t    PT_recovery;     //. Permet de récupérer PloadType des paq. média
  uint32_t   TS_recovery;     //. Permet de récupérer TimeStamp des paq. média
  sMediaNo   SNBase;  //. MédiaNo du 1er paquet média protégé
  sMediaNo   Offset;  //. MédiaNo médias protégés = SNBase + j*Offset
  sMediaNo   NA;      //. J est entre [0 ; NA[
  uint8_t    number;  //. Nombre de paquet média manquants
  sChampBits missing; //. Chaque bit = flag (perdu/non) d'un paquet média
  eFecD      D;       //. Direction : colonne ou ligne (col,row)
  uint8_t*   resXor;  //. Résultat de l'op. xor entre paquets média protégés
}
  sWaitFec;

// Déclaration des Fonctions ===================================================

sWaitFec* sWaitFec_New     (bool pInitParams);
sWaitFec* sWaitFec_Forge   (const sPaquetFec*);
void      sWaitFec_Release (      sWaitFec*);
void      sWaitFec_Print   (const sWaitFec*);

sMediaNx sWaitFec_GetManque (const sWaitFec*, uint8_t pNo);
bool     sWaitFec_SetManque (      sWaitFec*, uint8_t pNo, bool pValeur);
sMediaNx sWaitFec_ComputeJ  (const sWaitFec*, sMediaNo);
#endif
