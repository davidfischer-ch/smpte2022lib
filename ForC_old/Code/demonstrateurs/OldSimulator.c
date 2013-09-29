/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Old test bench
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

// Constantes de test (voir aussi types.h) =====================================

// Remarque : si les deux algorithmes sont actifs et que OPTION_VALIDATION est
//            définit alors buffer média traité par l'algorithme optimisé est
//            comparé à buffer média traité par l'algorithme de force brute en
//            vue de valider le premier.

//     <-----L (cols)----->
//
//  |  p 01 p 02 p 03 p 04  l 01      p nb = paquet nb média data
//  D  p 05 p 06 p 07 p 08  l 02      c nb = paquet nb FEC colonne
//  |  p 09 p 10 p 11 p 12  l 03      l nb = paquet nb FEC ligne
//
//     c 01 c 02 c 03 c 04

#define OPTION_VALIDATION /* Validation de l'algo via l'ensemble des cas ? */
//#define OPTION_DEMO       /* Démonstration visuelle du traitement ? */

const bool OPTION_DAVID = true; //. Utiliser l'algorithme optimisé ?
const bool OPTION_BRUTE = false; //. Utiliser l'algorithme brute    ?

const double OPTION_TEWFIQ_P = 0.001; //. Probabilité de passer de ok à perte /1
const double OPTION_TEWFIQ_Q = 0.5; //. Probabilité de passer de perte à ok /1

const unsigned OPTION_LRECOV = PLDS; //. Longueur du payload (resXor)
const sMediaNo OPTION_MEDIA0 = 0;    //. Premier no de séquence (initial)
const sMediaNo OPTION_BUFFER = 4000; //. Nb de media stockés avant lecture

const uint8_t  OPTION_L  = 10;      //. Taille (colonne) de la matrice de FEC
const uint8_t  OPTION_D  = 10;      //. Taille (ligne)   de la matrice de FEC
const unsigned OPTION_LD = 10*10;   //. Produit des deux paramètres (L*D)
const uint8_t  OPTION_DECALAGE = 0; //. Décalage constant sur colonne ?

// Variables Globales ==========================================================

static sTewfiq    tewfiq; //. Notre variable d'utilisation de tewfiq
static sDavidSmpte david; //. Notre variable d'utilisation de l'algo optimisé
static sBruteSmpte brute; //. Notre variable d'utilisation de l'algo force brute
static bool        init = false; //. Algorithmes initialisés ?

// Fonctions publiques =========================================================

// Affiche (et enregistre dans un fichier) le contenu des deux algorithmes -----
void AssertPrintError()
{
  if (!init) return;

  if (OPTION_DAVID) sDavidSmpte_Print (&david, true);
  if (OPTION_BRUTE) sBruteSmpte_Print (&brute, true);
}

// Point d'entrée du programme -------------------------------------------------
//> Code d'erreur renvoyé au système (0 = ok)
int main()
{
  unsigned no;

  //freopen ("SimulateurComplet.log", "w", stdout);

  PRINT1 ("\n--------------------------------\n"
          "\nFec Simulator by David Fischer !\n\n")

  ASSERT (OPTION_DAVID || OPTION_BRUTE,    -1, "DAVID or/and BRUTE must be set")
  ASSERT (OPTION_LD == OPTION_L * OPTION_D,-1, "LD must be equal to L * D")

  // SMPTE 2022 définit quelques limites standards
  ASSERT (OPTION_LD <= 100,                    -1,"SMPTE 2022 ... L * D <= 100")
  ASSERT ((OPTION_L >= 1) && (OPTION_L <= 20), -1,"SMPTE 2022 ... 1 <= L <= 20")
  ASSERT ((OPTION_D >= 4) && (OPTION_D <= 20), -1,"SMPTE 2022 ... 4 <= D <= 20")

  srand (clock()); // Si besoin de nombres aléatoires
  tewfiq = sTewfiq_New2 (OPTION_TEWFIQ_P, OPTION_TEWFIQ_Q);

  sChampBits configNo = sChampBits_New(); // Numéro de config en cours de test
  sChampBits limiteNo = sChampBits_New(); // Numéro de config limite (2^LD)

  sChampBits_SetBit (&limiteNo, OPTION_LD, true); // Impose la limite 2^LD

  // INITIALISATION DE SESSION MEDIA / FEC =====================================

  sMediaNo media0 = OPTION_MEDIA0;
  sFecNo   uniId  = 0;

  if (OPTION_DAVID) david = sDavidSmpte_New (false);
  if (OPTION_BRUTE) brute = sBruteSmpte_New (false);

  init = true;

  // DÉMARRAGE DE SESSION MEDIA / FEC ==========================================

  while (true)
  {
    #ifdef OPTION_VALIDATION

      PRINT2   ("\n============= configuration n° ")
      DETAILS2 (sChampBits_Print (&configNo);)
      PRINT2   ("=============\n\n")

    #endif

    sMediaNo mediaB = media0;

    // Simule l'arrivée des paquets média

    for (no = 0; no < OPTION_LD; no++)
    {
      sPaquetMedia *_mediaDavid, *_mediaBrute;

// FIXME
      /*if (OPTION_DAVID) _mediaDavid = sPaquetMedia_Forge (media0, 0, 0);
      if (OPTION_BRUTE) _mediaBrute = sPaquetMedia_Forge (media0, 0, 0);*/

      media0++;

      #ifdef OPTION_VALIDATION
      if (!sChampBits_GetBit (&configNo, no))
      {
        tewfiq.nombreOk++;
      #else
      if (sTewfiq_IsOkayOrLost (&tewfiq))
      {
      #endif
        if (OPTION_DAVID) sDavidSmpte_ArriveePaquetMedia (&david, _mediaDavid);
        if (OPTION_BRUTE) sBruteSmpte_ArriveePaquetMedia (&brute, _mediaBrute);
      }
      else
      {
      #ifdef OPTION_VALIDATION
        tewfiq.nombrePertes++;
      #endif
        if (OPTION_DAVID) sPaquetMedia_Release (_mediaDavid);
        if (OPTION_BRUTE) sPaquetMedia_Release (_mediaBrute);
      }
    }

    PRINT2 ("\n")

    // Simule l'arrivée des paquets de FEC ligne

    for (no = 0; /*no < OPTION_D*/; no++)
    {
      sMediaNo _SNBase = mediaB + no * OPTION_L;

      sPaquetFec* _fec = sPaquetFec_Forge
        (uniId, OPTION_LRECOV, _SNBase, 0, 0, OPTION_L, OPTION_D, ROW, 0);

      // TODO quand c'est modulo ... blem
      if (_fec->DWORD0.SNBase_low_bits +
          _fec->DWORD3.Offset * (_fec->DWORD3.NA-1) >= media0)
      {
        sPaquetFec_Release (_fec);
        break;
      }

      if (OPTION_DAVID)
      {
        sPaquetFec* _fecDavid = sPaquetFec_Copy (_fec);
        sDavidSmpte_ArriveePaquetFec (&david, _fecDavid);
      }

      if (OPTION_BRUTE)
      {
        sPaquetFec* _fecBrute = sPaquetFec_Copy (_fec);
        sBruteSmpte_ArriveePaquetFec (&brute, _fecBrute);
      }

      sPaquetFec_Release (_fec);
      uniId++;
    }

    PRINT2 ("\n")

    // Simule l'arrivée des paquets de FEC colonne

    for (no = 0; /*no < OPTION_L*/; no++)
    {
      //sMediaNo _mediaNo  = no   + nb * OPTION_L;
      sMediaNo _SNBase = mediaB + no * (1 + OPTION_L * OPTION_DECALAGE);

      sPaquetFec* _fec = sPaquetFec_Forge
      (uniId, OPTION_LRECOV, _SNBase, 0, 0, OPTION_L, OPTION_D, COL, 0);

      // TODO quand c'est modulo ... blem
      if (_fec->DWORD0.SNBase_low_bits +
          _fec->DWORD3.Offset * (_fec->DWORD3.NA-1) >= media0)
      {
        sPaquetFec_Release (_fec);
        break;
      }

      if (OPTION_DAVID)
      {
        sPaquetFec* _fecDavid = sPaquetFec_Copy (_fec);
        sDavidSmpte_ArriveePaquetFec (&david, _fecDavid);
      }

      if (OPTION_BRUTE)
      {
        sPaquetFec* _fecBrute = sPaquetFec_Copy (_fec);
        sBruteSmpte_ArriveePaquetFec (&brute, _fecBrute);
      }

      sPaquetFec_Release (_fec);
      uniId++;
    }

    if (OPTION_BRUTE) sBruteSmpte_AppliqueFec (&brute);

    #ifdef OPTION_VALIDATION

      if (OPTION_DAVID && OPTION_BRUTE)
      {
        if (david.media.rbtree.count != brute.media.rbtree.count)
        {
          PRINT1 ("avec la force brute t'es cuit %u, %u !\n",
                  david.media.rbtree.count, brute.media.rbtree.count)

          assert (david.media.rbtree.count == brute.media.rbtree.count);
        }
      }

    #endif

    // SIMULE LA LECTURE DE X PAQUETS MEDIA ====================================

    #ifndef OPTION_VALIDATION

      if (OPTION_DAVID)
      {
        while (true)
        {
          if (!sDavidSmpte_LecturePaquetMedia (&david, OPTION_BUFFER, NULL))
            break;
        }
      }

      if (OPTION_BRUTE)
      {
        while (true)
        {
          if (!sBruteSmpte_LecturePaquetMedia (&brute, OPTION_BUFFER, NULL))
            break;
        }
      }

    #endif

    // MESSAGE TOUS LES 0x1000 TRAITEMENTS =====================================

    #ifndef OPTION_DEMO
    if ((configNo.buffer[0] % 0x00001000) == 0)
    #endif
    {
      sChampBits_Print (&configNo); PRINT1 (" sur \n")
      sChampBits_Print (&limiteNo); PRINT1 ("\n\n")

      // Affiche les modes actifs
      PRINT1 ("modes : ")
      #ifdef OPTION_VALIDATION
      PRINT1 ("validation")
      #else
      PRINT1 ("tewfiq P=%.4g Q=%.4g", OPTION_TEWFIQ_P, OPTION_TEWFIQ_Q)
      #endif
      PRINT1 (" ")
      #ifdef OPTION_DEMO
      PRINT1 ("demo")
      #endif
      PRINT1 ("\n")

      // Affiche la configuration de test
      PRINT1 ("lrecov   = %u octets\n"
              "media0   = %u mediaNo\n"
              "buffered = %u paquets media\n",
              OPTION_LRECOV, OPTION_MEDIA0, OPTION_BUFFER)

      PRINT1 ("matrice L %u, D %u, decalage %u\n",
              OPTION_L, OPTION_D, OPTION_DECALAGE)

      clock_t cd = david.chronoFec / TICKS_TO_MS;
      clock_t cb = brute.chronoFec / TICKS_TO_MS;

      PRINT1 ("\n")
      if (OPTION_DAVID) PRINT1 ("david(%lu) ", cd)
      if (OPTION_BRUTE) PRINT1 ("brute(%lu) ", cb)

      PRINT1 ("ms")

      if (OPTION_DAVID && OPTION_BRUTE)
        PRINT1 (" g=%.4g x", (double)cb/(double)cd)

      PRINT1 ("\n\n")

      #ifndef OPTION_VALIDATION
        sTewfiq_Print (&tewfiq);
      #endif

      if (OPTION_DAVID) sDavidSmpte_Print (&david, false);
      if (OPTION_BRUTE) sBruteSmpte_Print (&brute, false);

      PRINT1 ("\n")

      #ifdef OPTION_DEMO

        unsigned noD, noL;

        for (noD = 0; noD < OPTION_D; noD++)
        {
          if (OPTION_DAVID)
          {
            for (noL = 0; noL < OPTION_L; noL++)
            {
              if (sBufferMedia_Find
                  (&david.media, mediaB + noL+noD*OPTION_L) != NULL)
                   PRINT1 ("D ")
              else PRINT1 (". ")
            }
          }

          if (OPTION_BRUTE)
          {
            PRINT1 ("    ")

            for (noL = 0; noL < OPTION_L; noL++)
            {
              if (sBufferMedia_Find
                  (&david.media, mediaB + noL+noD*OPTION_L) != NULL)
                   PRINT1 ("B ")
              else PRINT1 (". ")
            }
          }
          PRINT1 ("\n")
        }
        PRINT1 ("\n")

        _sleep (100);

      #endif
    }

    // INCRÉMENTATION DES COMPTEURS ============================================

    sChampBits_Incremente (&configNo);

    #ifdef OPTION_VALIDATION

      // Limite atteinte ?

      if (sChampBits_Compare (&configNo, &limiteNo) >= 0) break;

      // Réinitialise les algorithmes

      media0 = OPTION_MEDIA0;

      clock_t davidTotal = david.chronoTotal;
      clock_t davidMedia = david.chronoMedia;
      clock_t davidFec   = david.chronoFec;
      clock_t bruteTotal = brute.chronoTotal;
      clock_t bruteMedia = brute.chronoMedia;
      clock_t bruteFec   = brute.chronoFec;

      if (OPTION_DAVID) sDavidSmpte_Release (&david);
      if (OPTION_BRUTE) sBruteSmpte_Release (&brute);

      if (OPTION_DAVID) david = sDavidSmpte_New (false);
      if (OPTION_BRUTE) brute = sBruteSmpte_New (false);

      david.chronoTotal += davidTotal;
      david.chronoMedia += davidMedia;
      david.chronoFec   += davidFec;
      brute.chronoTotal += bruteTotal;
      brute.chronoMedia += bruteMedia;
      brute.chronoFec   += bruteFec;

    #endif
  }

  // FIN DE SESSION MEDIA / FEC ================================================

  if (OPTION_DAVID) sDavidSmpte_Release (&david);
  if (OPTION_BRUTE) sBruteSmpte_Release (&brute);

  return 0;
}
