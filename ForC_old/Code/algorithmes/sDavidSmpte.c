/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Optimized implementation of SMPTE 2022-1 FEC
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

#define MIN(a,b) (a <= b ? a : b)

// Déclaration de Fonctions privées ============================================

sCrossFec* sDavidSmpte_PerduPaquetMedia (sDavidSmpte*, sMediaNo, sWaitFec*);
void sDavidSmpte_RecupPaquetMedia (sDavidSmpte*,sMediaNo,sCrossFec*,sWaitFec*);

// Fonctions publiques =========================================================

// Création d'un nouveau David SMPTE                                       -----
// Remarque : ne pas oublier de faire le ménage avec sDavidSmpte_Release ! -----
//> Nouveau David SMPTE
sDavidSmpte sDavidSmpte_New
  (bool pOverwriteMedia) //: Ecrasage des doublons dans buffer média autorisé ?
{
  sDavidSmpte _david;

  _david.media                = sBufferMedia_New();
  _david.fec                  = sBufferFec_New();
  _david.overwriteMedia       = pOverwriteMedia;
  _david.recovered            = 0;
  _david.unrecoveredOnReading = 0;
  _david.nbArPaMedia          = 0;
  _david.nbArPaFec            = 0;
  _david.nbLePaMedia          = 0;
  _david.nbPePaMedia          = 0;
  _david.nbRePaMedia          = 0;
  _david.maxC                 = 0;
  _david.maxW                 = 0;
  _david.chronoTotal          = 0;
  _david.chronoMedia          = 0;
  _david.chronoFec            = 0;

  return _david;
}

// Libère la mémoire allouée par l'algorithme optimisé -------------------------
void sDavidSmpte_Release
  (sDavidSmpte* pDavid) //: David SMPTE à vider
{
  ASSERTpc (pDavid,, cExNullPtr)

  sBufferMedia_Release (&pDavid->media);
  sBufferFec_Release   (&pDavid->fec);
}

// Affiche le contenu d'un David SMPTE -----------------------------------------
void sDavidSmpte_Print
  (const sDavidSmpte* pDavid,   //: David SMPTE à afficher
   bool               pBuffers) //: Faut-il afficher les détails (mémoire vars?)
{
  ASSERTpc (pDavid,, cExNullPtr)

  PRINT1 (cMsgPrintDavidMedia)
  sBufferMedia_Print (&pDavid->media, pBuffers);

  PRINT1 (cMsgPrintDavidCross)
  sBufferFec_PrintCross (&pDavid->fec, pBuffers);

  PRINT1 (cMsgPrintDavidWaitCol)
  sBufferFec_PrintWait (&pDavid->fec, COL, pBuffers);

  PRINT1 (cMsgPrintDavidWaitRow)
  sBufferFec_PrintWait (&pDavid->fec, ROW, pBuffers);

  clock_t ct = pDavid->chronoTotal / TICKS_TO_MS;
  clock_t cm = pDavid->chronoMedia / TICKS_TO_MS;
  clock_t cf = pDavid->chronoFec   / TICKS_TO_MS;

  PRINT1 (cMsgPrintDavid,
          pDavid->overwriteMedia ? cMsgOverwriteMediaYes : cMsgOverwriteMediaNo,
          pDavid->media.rbtree.overCount,
          pDavid->recovered,
          pDavid->unrecoveredOnReading,
          pDavid->media.readingNx.v,
          pDavid->media.arrivalNx.v,
          ct, cm, cf,
          pDavid->nbArPaMedia,
          pDavid->nbArPaFec,
          pDavid->nbLePaMedia,
          pDavid->nbPePaMedia,
          pDavid->nbRePaMedia,
          pDavid->maxC,
          pDavid->maxW)
}

/*******************************************************************************
*                    GÈRE L'ARRIVÉE DE PAQUETS (MEDIA ou FEC)                  *
*******************************************************************************/

// Un paquet média vient d'arriver (ou d'être récupéré), tente de trouver    ---
// une entrée correspondante dans le buffer de FEC (via médiaNo). Si celle-  ---
// ci existe alors il faut faire appel au mécanisme de récuperation pour     ---
// mettre à jour buffer de FEC et peut-être débloquer la cascade de          ---
// récupération à partir de paquets FEC qui étaient jusque là en attente     ---
void sDavidSmpte_ArriveePaquetMedia
  (sDavidSmpte * pDavid, //: David SMPTE à mettre à jour
   sPaquetMedia* pMedia) //: Paquet média arrivant (du réseau)
{
  ASSERTpc (pDavid,, cExNullPtr)
  ASSERTpc (pMedia,, cExNullPtr)

  PRINT2 ("David ArriveePaquetMedia mediaNo=%u : ", pMedia->mediaNo)

  clock_t add = 0;
  clock_t now = clock();

  bool ok = sBufferMedia_AddByReference
              (&pDavid->media, pMedia, pDavid->overwriteMedia);
  ASSERT (ok,, cExMediaAdd, pMedia->mediaNo)

  add = clock() - now;
  pDavid->chronoTotal += add;
  pDavid->chronoMedia += add;
  now = clock();

  // Le paquet média est signalé comme perdu dans FEC : simuler la récup. !
  sCrossFec* _cross = sBufferFec_FindCross (&pDavid->fec, pMedia->mediaNo);

  if (_cross != 0)
  {
    PRINT2 ("le paquet media est cité dans bufferFec.cross\n")

    sDavidSmpte_RecupPaquetMedia (pDavid, pMedia->mediaNo, _cross, 0);
  }
  else
  {
    // Aucune entrée trouvée ... rien faire de particulier
    PRINT2 ("aucun paquet de FEC ne cite ce paquet media\n")
  }

  add = clock() - now;
  pDavid->nbArPaMedia++;
  pDavid->chronoTotal += add;
  pDavid->chronoFec   += add;
}

// Un paquet de FEC vient d'arriver, liste les paquets média manquants que   ---
// le nouveau paquet de FEC est capable de récupérer.                        ---
// Dès lors 3 cas de figures peuvent arriver au paquet de FEC :              ---
// [1] Inutile (aucun des paquets média protégés ne manque) > jeté           ---
// [2] Récupère le seul paquet média manquant > opération exécutée puis jeté ---
// [3] Bloqué car >1 paquets média manquent> stocké pour cascade future      ---
void sDavidSmpte_ArriveePaquetFec
  (sDavidSmpte* pDavid, //: David SMPTE à mettre à jour
   sPaquetFec * pFec)   //: Paquet de FEC arrivant (du réseau)
{
  ASSERTpc (pDavid,, cExNullPtr)
  ASSERTpc (pFec,,   cExNullPtr)

  PRINT2   ("David ArriveePaquetFec ")
  DETAILS2 (sPaquetFec_Print (pFec);)
  PRINT2   ("\n")

  clock_t add = 0;
  clock_t now = clock();

  // Lecture des champs du paquet SMPTE 2022-1 FEC

  if (pFec->DWORD1.Mask  != FEC_MASK_0)    return; // doit être 0
  if (pFec->DWORD3.X     != FEC_X_0)       return; // doit être 0
  if (pFec->DWORD3.type  != XOR)           return; // doit être XOR
  if (pFec->DWORD3.index != FEC_INDEX_XOR) return; // doit être 0

  sWaitFec* _wait = sWaitFec_Forge (pFec);
  ASSERTc  (_wait,, cExFecForge)

  sPaquetFec_Release (pFec);

  sCrossFec* _crossLast = 0;
  sMediaNo   _mediaLast = 0;
  sMediaNo   _mediaTest = _wait->SNBase;
  sMediaNo   _mediaMax  = _wait->SNBase + _wait->NA * _wait->Offset;

  // Paquet média protégés : médiaNo = SNBase + j*offset, avec j entre [0;NA[
  for (; _mediaTest != _mediaMax; _mediaTest += _wait->Offset)
  {
    if (sBufferMedia_Find (&pDavid->media, _mediaTest) != NULL)
    {
      PRINT2 (cMsgDavidArPaFecPresent, _mediaTest)
    }
    else
    {
      PRINT2 (cMsgDavidArPaFecMissing, _mediaTest)

      _crossLast =
        sDavidSmpte_PerduPaquetMedia (pDavid, _mediaLast = _mediaTest, _wait);
    }
  }

  // [1] Aucune entrée enregistrée : paquet de FEC inutile à conserver

  if (_wait->number == 0)
  {
    PRINT2 ("David ArriveePaquetFec : paquet de FEC est inutile\n\n")

    sWaitFec_Release (_wait);
    goto __fin_chrono;
  }

  // Enregistre le paquet de FEC dans bufferFec.wait[D]

  bool    ok = sBufferFec_AddWaitByReference (&pDavid->fec, _wait, false);
  ASSERT (ok,, cExWaitAdd, _wait->fecNo)

  if (pDavid->fec.wait[_wait->D].count > pDavid->maxW)
  {
    pDavid->maxW = pDavid->fec.wait[_wait->D].count;
  }

  // [2] Qu'un seul paquet média manquant : récupération possible

  if (_wait->number == 1)
  {
    PRINT2("David ArriveePaquetFec : paquet de FEC va recuperer paquet media\n")

    sDavidSmpte_RecupPaquetMedia (pDavid, _mediaLast, _crossLast, _wait);
  }

  // [3] Récupération impossible pour l'instant, conserver le paquet de fec

  else
  {
    PRINT2
    ("David ArriveePaquetFec : paquet de FEC conserve pour cascade future\n\n")
  }

__fin_chrono:
  add = clock() - now;
  pDavid->nbArPaFec++;
  pDavid->chronoTotal += add;
  pDavid->chronoFec   += add;
}

// Imite la lecture du buffer média et en profite pour nettoyer les buffers ----
//> Est-ce qu'une lecture a eu lieu ?
bool sDavidSmpte_LecturePaquetMedia
  (sDavidSmpte* pDavid,      //: David SMPTE à mettre à jour
   sMediaNo     pBufferSize, //: Nombre de paquets média à garder dans le buffer
   FILE*        pDestFile)   //: Pour enregistrer le payload média (0=pas enreg)
{
  ASSERTpc (pDavid, false, cExNullPtr)

  // Le buffer média n'a pas dépassé la capacité demandée
  if (pDavid->media.rbtree.count <= pBufferSize) return false;

  clock_t add = 0;
  clock_t now = clock();

  sMediaNo _readedNo;

  bool ok = sBufferMedia_ReadMedia (&pDavid->media, pDestFile, &_readedNo);

  add = clock() - now;
  pDavid->chronoTotal += add;
  pDavid->chronoMedia += add;
  now = clock();

  PRINT2 ("David LecturePaquetMedia %u\n", _readedNo)

  // Nettoye le buffer de FEC

  sCrossFec* _cross = sBufferFec_FindCross (&pDavid->fec, _readedNo);

  if (_cross)
  {
    // Un paquet de FEC dépendant d'un paquet média supprimé est inutile car
    // l'opération de récuperation ne peut réussire (xor)

    sFecNx _colNx = _cross->colNx;
    sFecNx _rowNx = _cross->rowNx;

    // TODO -> optimisation (éviter doublons de code ?)

    if (!_colNx.null)
    {
      // Utilise wait pour retrouver les entrées dans cross qui doivent
      // êtres supprimées (nettoyage)

      sBufferFec_DeleteCrossAndWait (&pDavid->fec, COL, _colNx.v);
    }

    if (!_rowNx.null)
    {
      // Utilise wait pour retrouver les entrées dans cross qui doivent
      // êtres supprimées (nettoyage)

      sBufferFec_DeleteCrossAndWait (&pDavid->fec, ROW, _rowNx.v);
    }
  }

  add = clock() - now;
  pDavid->nbLePaMedia++;
  if (!ok) pDavid->unrecoveredOnReading++;
  pDavid->chronoTotal += add;
  pDavid->chronoFec   += add;

  return true;
}

/*******************************************************************************
*                  COEUR DE L'ALGORITHME DE FEC SMPTE 2022-1                     *
*******************************************************************************/

// Enregistre l'info comme quoi un paquet média manque et le lie au paquet   ---
// FEC ayant remarqué que le dit paquet média était manquant !               ---
// Attention : n'enregistre pas dans bufferFec.wait[D] (le fait plus tard)   ---
sCrossFec* sDavidSmpte_PerduPaquetMedia
  (sDavidSmpte* pDavid,   //: David SMPTE à mettre à jour
   sMediaNo     pMediaNo, //: MédiaNo du paquet média manquant
   sWaitFec*    pWait)    //: Wait (FEC) ayant remarqué la perte
{
  ASSERTpc (pDavid, 0, cExNullPtr)
  ASSERTpc (pWait,  0, cExNullPtr)

  PRINT2   ("David PerduPaquetMedia mediaNo=%u\n", pMediaNo)
  DETAILS2 (sWaitFec_Print (pWait);)
  PRINT2   ("\n")

  // ajoute la nouvelle entrée [lie paquet média manquant <-> paquet de FEC]

  // Recherche si l'entrée est déjà existante ...
  sCrossFec* _cross = sBufferFec_FindCross (&pDavid->fec, pMediaNo);

  if (_cross == 0)
  {
    _cross = sCrossFec_New();
    ASSERTc (_cross, 0, cExCrossNew)

    bool ok =
      sBufferFec_AddCrossByReference (&pDavid->fec, pMediaNo, _cross, false);
    ASSERT (ok, 0, cExCrossAdd, pMediaNo)

    if (pDavid->fec.cross.count > pDavid->maxC)
    {
      pDavid->maxC = pDavid->fec.cross.count;
    }
  }

  // Met à jour l'élément dans pBufferFec.cross

  if (pWait->D == COL)
  {
    ASSERTc (_cross->colNx.null, 0, cExAlgorithmCaller)

    _cross->colNx = sFecNo_to_sFecNx (pWait->fecNo);
  }
  else
  {
    ASSERTc (_cross->rowNx.null, 0, cExAlgorithmCaller)

    _cross->rowNx = sFecNo_to_sFecNx (pWait->fecNo);
  }

  sMediaNx j = sWaitFec_ComputeJ (pWait, pMediaNo);
  ASSERTc (!j.null, 0, cExWaitComputeJ)

  // Enregistre que le paquet média médiaNo = (no_bit_dans_tab_manque) manque
  bool     ok = sWaitFec_SetManque (pWait, j.v, true);
  ASSERTc (ok, 0, cExWaitSetManque)

  pDavid->nbPePaMedia++;

  return _cross;
}

// Un paquet média vient d'arriver (ou est récupéré), il faut donc m-à-j la  ---
// structure buffer de FEC et éventuel. activer d'autres FEC en cascade !    ---
// Cette fonction peut-être appelée pour gérer 3 cas de figures différents : ---
// [1] "Mise à jour" du buffer de FEC car un paquet média est arrivé         ---
// [2] Récuperation car 1 seul paquet média manquant                         ---
// [3] Cascade de FEC réalisable !                                           ---
void sDavidSmpte_RecupPaquetMedia
  (sDavidSmpte* pDavid,   //: David SMPTE à mettre à jour
   sMediaNo     pMediaNo, //: médiaNo du paquet média à récupérer (ou récupéré)
   sCrossFec*   pCross,   //: Cross du paquet média à récupérer (ou récupéré)
   sWaitFec*    pWait)    //: Wait (FEC) ayant remarqué la perte (si par FEC)
{
  ASSERTpc (pDavid,, cExNullPtr)
  ASSERTpc (pCross,, cExNullPtr)

  bool _parFec = pWait != 0; // [2 ou 3]

  PRINT2   ("David RecupPaquetMedia ")
  DETAILS2 (if (_parFec) { PRINT2 ("[2 ou 3] ") }
            else         { PRINT2 ("[1] ") }
            PRINT2 ("%u ", pMediaNo)
            if (_parFec) { sCrossFec_Print (pCross); }
            if (_parFec) { sWaitFec_Print  (pWait);  })
  PRINT2   ("\n")

  unsigned no;

  // [1 ou 2 ou 3] Lecture des données du cross et suppression de ce dernier

  sFecNx _cascadeFecNx[2];
  _cascadeFecNx[COL] = pCross->colNx;
  _cascadeFecNx[ROW] = pCross->rowNx;

  bool     ok = sBufferFec_DeleteCross (&pDavid->fec, pMediaNo);
  ASSERTc (ok,, cExCrossDelete)

  // [2 ou 3] Récupère le paquet média
  //          Supprime les trace du paquet de FEC devenu inutile

  if (_parFec)
  {
    ASSERTc (pWait->number == 1,,                        cExAlgorithmCaller)
    ASSERTc (!_cascadeFecNx[pWait->D].null,,             cExAlgorithmCaller)
    ASSERTc (pWait->fecNo == _cascadeFecNx[pWait->D].v,, cExAlgorithmCaller)

    PRINT2 (cMsgDavidRePaMediaRecover, pMediaNo)

    // Etapes de la récupération (2 étapes) :
    // > payloadRecup = paquetFec.resXor

    sPaquetMedia* _recup = sPaquetMedia_Forge
        (pMediaNo, pWait->TS_recovery,     pWait->PT_recovery,
                   pWait->Length_recovery, pWait->resXor);
    ASSERTc (_recup,, cExMediaForge)

    // > payloadRecup ^= (tous les paquetMedia liés au paquetFec)

    sMediaNo _mediaNo  = pWait->SNBase;
    sMediaNo _mediaMax = pWait->SNBase + pWait->NA * pWait->Offset;

    // Paquet média protégés : médiaNo = SNBase + j*offset, avec j entre [0;NA[
    for (; _mediaNo != _mediaMax; _mediaNo += pWait->Offset)
    {
      if (_mediaNo == pMediaNo) continue;

      sPaquetMedia* _ami = sBufferMedia_Find (&pDavid->media, _mediaNo);

      _recup->timeStamp   ^= _ami->timeStamp;
      _recup->payloadType ^= _ami->payloadType;

      unsigned _size = MIN (_recup->payloadSize, _ami->payloadSize);
      for (no = 0; no < _size; no++)
      {
        _recup->payload[no] ^= _ami->payload[no];
      }
    }

    bool ok = sBufferMedia_AddByReference
                (&pDavid->media, _recup, pDavid->overwriteMedia);
    ASSERT (ok,, cExMediaAdd, _recup->mediaNo)

    pDavid->recovered++;

    _cascadeFecNx[pWait->D] = FEC_NX_NULL;

    ok = sBufferFec_DeleteWait (&pDavid->fec, pWait->D, pWait->fecNo);
    ASSERTc (ok,, cExWaitDelete)
  }

  // [1 ou 2 ou 3] Vérifie s'il y a une cascade ...

  sWaitFec* _cascadeWait[2] = {0, 0};

  for (no = 0; no < 2; no++)
  {
    if (_cascadeFecNx[no].null) continue;

    // Recherche dans buffer de FEC si le paquet de FEC lié au paquet média
    // vient de se faire débloquer (il attendait sur 2 paquet média et
    // maintenant sur un seul) et dans ce cas, l'activerait en cascade ...

    _cascadeWait[no] =
      sBufferFec_FindWait (&pDavid->fec, no, _cascadeFecNx[no].v);
    ASSERTc (_cascadeWait[no],, cExFecFindWait)

    sMediaNx j = sWaitFec_ComputeJ (_cascadeWait[no], pMediaNo);
    ASSERTc (!j.null,, cExWaitComputeJ)

    // Ne retire que le paquet média médiaNo = (no_bit_dans_tab_manque) manque
    sWaitFec_SetManque (_cascadeWait[no], j.v, false);
  }

  for (no = 0; no < 2; no++)
  {
    if (_cascadeFecNx[no].null) continue;

    if (_cascadeWait[no]->number == 1)
    {
      // Cascade !
      PRINT2 ("RecupPaquetMedia : Cascade !\n")

      sMediaNx _cascadeMediaNx = sWaitFec_GetManque (_cascadeWait[no], 1);
      ASSERTc (!_cascadeMediaNx.null,, cExWaitComputeNo)

      sCrossFec* _cascadeCross =
        sBufferFec_FindCross (&pDavid->fec, _cascadeMediaNx.v);
      ASSERT (_cascadeCross,, cExFecFindCross, _cascadeMediaNx.v)

      sDavidSmpte_RecupPaquetMedia
        (pDavid, _cascadeMediaNx.v, _cascadeCross, _cascadeWait[no]);
    }
  }

  PRINT2   ("David RecupPaquetMedia ")
  DETAILS2 (if (_parFec) { PRINT2 ("[2 ou 3] ") }
            else         { PRINT2 ("[1] ") }
            PRINT2 ("%u fin\n", pMediaNo))

  pDavid->nbRePaMedia++;
}
