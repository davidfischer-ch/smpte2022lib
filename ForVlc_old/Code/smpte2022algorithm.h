
/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Main Developer : David Fischer (david.fischer.ch@gmail.com)
    VLC Integrator : Jérémie Rossier (jeremie.rossier@gmail.com)
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

#ifndef __SDAVIDSMPTE__
#define __SDAVIDSMPTE__

// Types de données ============================================================

// Structure stockant ce qu'il faut pour l'algorithme de SMPTE 2022-1 optimisé ---
typedef struct sDavidSmpte
{
  sBufferMedia_t *media; //. Stockage des paquets de média
  sBufferFec_t   *fec;   //. Stockage des paquets de FEC et FEC <-> média

  bool resized_matrix; //savoir si la matrice a été redimentionnée
  mtime_t resized_matrix_check;
  unsigned recovered;            //. Nombre de paquets média récupérés
  unsigned unrecoveredOnReading; //. Nb paq. média manquants lors de la lecture !

  unsigned nbArPaMedia; //. Nombre d'appels à ArriveePaquetMedia
  unsigned nbArPaFec;   //. Nombre d'appels à ArriveePaquetFec
  unsigned nbLePaMedia; //. Nombre d'appels à LecturePaquetMedia
  unsigned nbPePaMedia; //. Nombre d'appels à PerduPaquetMedia
  unsigned nbRePaMedia; //. Nombre d'appels à RecupPaquetMedia

  unsigned maxC; //. Nombre max d'éléments stockés dans Cross
  unsigned maxW; //. Nombre max d'éléments stockés dans Wait

}sDavidSmpte_t;

// Déclaration des fonctions ===================================================

sDavidSmpte_t *sDavidSmpte_New ();
void sDavidSmpte_Release (sDavidSmpte_t *);

bool sDavidSmpte_ArriveePaquetMedia (demux_t *demux, sDavidSmpte_t*, sPaquetMedia*);
bool sDavidSmpte_ArriveePaquetMedia_Convert(demux_t *demux, sDavidSmpte_t*, block_t*);
void sDavidSmpte_ArriveePaquetFec   (demux_t *demux, sDavidSmpte_t*, sPaquetFec*);
void sDavidSmpte_ArriveePaquetFec_Convert(demux_t *demux, sDavidSmpte_t*, block_t*);
bool sDavidSmpte_LecturePaquetMedia (demux_t *demux, sDavidSmpte_t*, sMediaNo);

sCrossFec *sDavidSmpte_PerduPaquetMedia (demux_t *demux,sDavidSmpte_t*,sMediaNo,sWaitFec*);
void sDavidSmpte_RecupPaquetMedia       (demux_t *demux,sDavidSmpte_t*,sMediaNo,sCrossFec*,sWaitFec*);

#endif
