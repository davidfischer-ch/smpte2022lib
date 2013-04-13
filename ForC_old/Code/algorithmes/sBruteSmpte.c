/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : Brute force implementation of SMPTE 2022-1 FEC
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

#define MIN(a,b) (a <= b ? a : b)

// Fonctions publiques =========================================================

void ReleasePaquetFunc (void* pValue)
{
  if (pValue) sPaquetFec_Release (pValue);
}

void PrintPaquetFunc (void* pValue)
{
  if (pValue) sPaquetFec_Print (pValue);
}

// Création d'un nouveau Brute SMPTE                                       -----
// Remarque : ne pas oublier de faire le ménage avec sBruteSMPTE_Release ! -----
//> Nouveau Brute SMPTE
sBruteSmpte sBruteSmpte_New
  (bool pOverwriteMedia) //: Ecrasage des doublons dans buffer média autorisé ?
{
  sBruteSmpte _brute;

  _brute.media = sBufferMedia_New();
  _brute.fec   = sLinkedList_New (ReleasePaquetFunc, PrintPaquetFunc);
  _brute.overwriteMedia       = pOverwriteMedia;
  _brute.recovered            = 0;
  _brute.unrecoveredOnReading = 0;
  _brute.nbArPaMedia          = 0;
  _brute.nbArPaFec            = 0;
  _brute.nbLePaMedia          = 0;
  _brute.nbApFec              = 0;
  _brute.maxF                 = 0;
  _brute.chronoTotal          = 0;
  _brute.chronoMedia          = 0;
  _brute.chronoFec            = 0;

  return _brute;
}

// Libère la mémoire allouée par l'algorithme de force brute -------------------
void sBruteSmpte_Release
  (sBruteSmpte* pBrute) //: Brute SMPTE à vider
{
  ASSERTpc (pBrute,, cExNullPtr)

  sBufferMedia_Release (&pBrute->media);
  sLinkedList_Release  (&pBrute->fec);
}

// Affiche le contenu d'un Brute SMPTE -----------------------------------------
void sBruteSmpte_Print
  (const sBruteSmpte* pBrute,   //: Brute SMPTE à afficher
   bool               pBuffers) //: Faut-il afficher les détails (mémoire vars?)
{
  ASSERTpc (pBrute,, cExNullPtr)

  PRINT1 (cMsgPrintBruteMedia)
  sBufferMedia_Print (&pBrute->media, pBuffers);

  PRINT1 (cMsgPrintBruteFec)
  sLinkedList_Print (&pBrute->fec, pBuffers);

  clock_t ct = pBrute->chronoTotal / TICKS_TO_MS;
  clock_t cm = pBrute->chronoMedia / TICKS_TO_MS;
  clock_t cf = pBrute->chronoFec   / TICKS_TO_MS;

  PRINT1 (cMsgPrintBrute,
          pBrute->overwriteMedia ? cMsgOverwriteMediaYes : cMsgOverwriteMediaNo,
          pBrute->media.rbtree.overCount,
          pBrute->recovered,
          pBrute->unrecoveredOnReading,
          pBrute->media.readingNx.v,
          pBrute->media.arrivalNx.v,
          ct, cm, cf,
          pBrute->nbArPaMedia,
          pBrute->nbArPaFec,
          pBrute->nbLePaMedia,
          pBrute->nbApFec,
          pBrute->maxF)
}
/*******************************************************************************
*                    GÈRE L'ARRIVÉE DE PAQUETS (MEDIA ou FEC)                  *
*******************************************************************************/

// Gère l'arrivée d'un paquet média --------------------------------------------
void sBruteSmpte_ArriveePaquetMedia
  (sBruteSmpte * pBrute, //: Brute SMPTE à mettre à jour
   sPaquetMedia* pMedia) //: Paquet média arrivant (du réseau)
{
  ASSERTpc (pBrute,, cExNullPtr)
  ASSERTpc (pMedia,, cExNullPtr)

  PRINT2 ("Brute ArriveePaquetMedia mediaNo=%u\n", pMedia->mediaNo)

  clock_t add = 0;
  clock_t now = clock();

  bool ok = sBufferMedia_AddByReference
              (&pBrute->media, pMedia, pBrute->overwriteMedia);
  ASSERT (ok,, cExMediaAdd, pMedia->mediaNo)

  add = clock() - now;
  pBrute->nbArPaMedia++;
  pBrute->chronoTotal += add;
  pBrute->chronoMedia += add;
}

// Gère l'arrivée d'un paquet de FEC -------------------------------------------
void sBruteSmpte_ArriveePaquetFec
  (sBruteSmpte* pBrute, //: Brute SMPTE à mettre à jour
   sPaquetFec * pFec)   //: Paquet de FEC arrivant (du réseau)
{
  ASSERTpc (pBrute,, cExNullPtr)
  ASSERTpc (pFec,,   cExNullPtr)

  PRINT2   ("Brute ArriveePaquetFec ")
  DETAILS2 (sPaquetFec_Print (pFec);)
  PRINT2   ("\n")

  clock_t add = 0;
  clock_t now = clock();

  sLinkedList_AppendByReference (&pBrute->fec, pFec);

  if (pBrute->fec.count > pBrute->maxF)
  {
     pBrute->maxF = pBrute->fec.count;
  }

  add = clock() - now;
  pBrute->nbArPaFec++;
  pBrute->chronoTotal += add;
  pBrute->chronoFec   += add;
}

// Imite la lecture du buffer média et en profite pour nettoyer les buffers ----
//> Est-ce qu'une lecture a eu lieu ?
bool sBruteSmpte_LecturePaquetMedia
  (sBruteSmpte* pBrute,      //: Brute SMPTE à mettre à jour
   sMediaNo     pBufferSize, //: Nombre de paquets média à garder dans le buffer
   FILE*        pDestFile)   //: Pour enregistrer le payload média (0=pas enreg)
{
  ASSERTpc (pBrute, false, cExNullPtr)

  // Le buffer média n'a pas dépassé la capacité demandée
  if (pBrute->media.rbtree.count <= pBufferSize) return false;

  clock_t add = 0;
  clock_t now = clock();

  sMediaNo _readedNo;

  bool ok = sBufferMedia_ReadMedia (&pBrute->media, pDestFile, &_readedNo);

  add = clock() - now;
  pBrute->chronoTotal += add;
  pBrute->chronoMedia += add;
  now = clock();

  PRINT2 ("Brute LecturePaquetMedia %u\n", _readedNo)

  // Nettoye le buffer de FEC

  if (sLinkedList_InitForeach (&pBrute->fec, false))
  {
    while (sLinkedList_NextForeach (&pBrute->fec))
    {
      sPaquetFec* _fec = sLinkedList_ForeachValue (&pBrute->fec);
      ASSERTc    (_fec, false, cExLinkedListValue)

      sMediaNo _SNBase = _fec->DWORD0.SNBase_low_bits/*+
                        _fec->DWORD3.SNBase_ext_bits*256*256*/;
      sMediaNo _Offset = _fec->DWORD3.Offset;
      sMediaNo _NA     = _fec->DWORD3.NA;

      sMediaNo _mediaNo  = _SNBase;
      sMediaNo _mediaMax = _SNBase + _NA * _Offset;

      // Recherche si le paquet de FEC corrigerait un paquet entre les bornes
      for (; _mediaNo != _mediaMax; _mediaNo += _Offset)
      {
        if (sBufferMedia_IsMediaNoInBuffer (&pBrute->media, _mediaNo))
        {
          goto __ok;
        }
      }

      // Paquet de FEC inutile

      bool     ok = sLinkedList_DeleteOnForeach (&pBrute->fec);
      ASSERTc (ok, false, cExFecDelete)

      // Paquet de FEC utile
      __ok:;
    }
  }

  add = clock() - now;
  pBrute->nbLePaMedia++;
  if (!ok) pBrute->unrecoveredOnReading++;
  pBrute->chronoTotal += add;
  pBrute->chronoFec   += add;

  return true;
}

/*******************************************************************************
*                  COEUR DE L'ALGORITHME DE FEC SMPTE 2022-1                     *
*******************************************************************************/

// Applique l'algorithme de force brute (récupération des paquets média) -------
void sBruteSmpte_AppliqueFec
  (sBruteSmpte* pBrute) //: Brute SMPTE à mettre à jour
{
  ASSERTpc (pBrute,, cExNullPtr)

  clock_t add = 0;
  clock_t now = clock();

  unsigned no;

  if (sLinkedList_InitForeach (&pBrute->fec, false))
  {
    while (sLinkedList_NextForeach (&pBrute->fec))
    {
      sPaquetFec* _fec = sLinkedList_ForeachValue (&pBrute->fec);
      ASSERTc    (_fec,, cExLinkedListValue)

      unsigned _Length_recovery = _fec->DWORD0.Length_recovery;
      sMediaNo _SNBase = _fec->DWORD0.SNBase_low_bits/*+
                         _fec->DWORD3.SNBase_ext_bits*256*256*/;
      sMediaNo _NA     = _fec->DWORD3.NA;
      sMediaNo _Offset = _fec->DWORD3.Offset;
      uint32_t _TS_recovery = _fec->DWORD2.TS_recovery;
      uint8_t  _PT_recovery = _fec->DWORD1.PT_recovery;
      //unsigned _D    = _fec->DWORD3.D;
      //uint8_t* _resXor = _fec->resXor;
      unsigned _nbMiss = 0;

      // Recherche les paquets Media manquants
      sMediaNo _mediaLast = 0;
      sMediaNo _mediaNo   = _SNBase;
      sMediaNo _mediaMax  = _SNBase + _NA * _Offset;

      // Paquet média protégés : médiaNo = SNBase + j*offset, avec j entre [0;NA[
      for (; _mediaNo != _mediaMax; _mediaNo += _Offset)
      {
        if (sBufferMedia_Find (&pBrute->media, _mediaNo) != NULL)
        {
          PRINT2 (cMsgBruteApFecPresent, _mediaNo)
        }
        else
        {
          PRINT2 (cMsgBruteApFecMissing, _mediaNo)

          _mediaLast = _mediaNo;

          if (++_nbMiss > 1) break;
        }
      }

      // Supprime le paquet de FEC (devenu) inutile !
      if (_nbMiss == 0)
      {
        sLinkedList_DeleteOnForeach (&pBrute->fec);
      }
      // Récupération possible
      else if (_nbMiss == 1)
      {
        PRINT2 (cMsgBruteApFecRecover, _mediaLast)

        // Etapes de la récupération (2 étapes) :
        // > payloadRecup = paquetFec.resXor

        sPaquetMedia* _recup = sPaquetMedia_Forge
          (_mediaLast, _TS_recovery,     _PT_recovery,
                       _Length_recovery, _fec->resXor);

        ASSERTc (_recup,, cExMediaForge)

        // > payloadRecup ^= (tous les paquetMedia liés au paquetFec)

        _mediaNo  = _SNBase;
        _mediaMax = _SNBase + _NA * _Offset;

        // Paquet média protégés :
        // médiaNo = SNBase + j*offset, avec j entre [0;NA[
        for (; _mediaNo != _mediaMax; _mediaNo += _Offset)
        {
          if (_mediaNo == _mediaLast) continue;

          sPaquetMedia* _ami = sBufferMedia_Find (&pBrute->media, _mediaNo);

          _recup->timeStamp   ^= _ami->timeStamp;
          _recup->payloadType ^= _ami->payloadType;

          unsigned _size = MIN (_recup->payloadSize, _ami->payloadSize);
          for (no = 0; no < _size; no++)
          {  
            _recup->payload[no] ^= _ami->payload[no];
          }
        }

        sBufferMedia_AddByReference
          (&pBrute->media, _recup, pBrute->overwriteMedia);

        pBrute->recovered++;

        // Recommence le scan du début ( ça c de la force brute ! )
        sLinkedList_DeleteOnForeach (&pBrute->fec);
        sLinkedList_InitForeach     (&pBrute->fec, false);
      }
    }
  }

  add = clock() - now;
  pBrute->nbApFec++;
  pBrute->chronoTotal += add;
  pBrute->chronoFec   += add;
}
