
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>
#include <assert.h>
#include <errno.h>

#include <vlc_common.h>

#include "smpte2022.h"
#include "smpte2022fec.h"

//------------------------------------------------------------------------------
//              PAQUET FEC
//------------------------------------------------------------------------------

// Constantes privées ==========================================================

const sPaquetFec INIT_PAQUET_FEC = //. Valeur par défaut d'un paquet de FEC (0)
  {0, {0,0}, {0,0,0}, {0}, {0,0,0,0,0,0,0}, 0};

// Constantes publiques ========================================================
const sFecNx   FEC_NX_NULL  ={0, true}; //. Valeur par défaut de fecNx (~null)

// Fonctions publiques =========================================================

// Création d'un nouveau paquet de FEC                                    ------
// Remarque : si OPTION_OVERWRITE_FEC_NO est actif alors fecNo du paquet  ------
// créé ne prendra pas en compte le paramètre pFecNo mais sera généré     ------
// Remarque : ne pas oublier de faire le ménage avec sPaquetFec_Release ! ------
//> Pointeur sur le nouveau paquet de FEC ou 0 si problème
sPaquetFec* sPaquetFec_Forge
  (sFecNo    pFecNo,           //: FecNo du paquet de FEC
   uint16_t  pLength_recovery, //: Longueur du payload (pResXor)
   sMediaNo  pSNBase,          //: Premier médiaNo protégé
   uint8_t   pPT_recovery,     //: Permet de récupérer PloadType des paq. média
   uint32_t  pTS_recovery,     //: Permet de récupérer TimeStamp des paq. média
   uint8_t   pL_cols,          //: Taille de la matrice de FEC (paramètre L)
   uint8_t   pD_rows,          //: Taille de la matrice de FEC (paramètre D)
   eFecD     pD,               //: Direction : colonne ou ligne (col,row)
   const uint8_t* pResXor) //: Résultat du xor entre paquets média protégés
{
  #ifdef OPTION_OVERWRITE_FEC_NO
  static fecNo_t fecNoUnique = 0;
  #endif

  pSNBase &= FEC_SNBASE_MASK;

  sPaquetFec* _fec = malloc (sizeof (sPaquetFec));
  if (_fec == NULL) // Allocation ratée ?
    return NULL;

  #ifdef OPTION_OVERWRITE_FEC_NO
  _fec->fecNo = fecNoUnique++;
  #else
  _fec->fecNo = pFecNo;
  #endif

  _fec->DWORD0.Length_recovery = pLength_recovery;
  _fec->DWORD0.SNBase_low_bits = pSNBase & 0x0000FFFF;
  _fec->DWORD1.Mask            = FEC_MASK_0;
  _fec->DWORD1.PT_recovery     = pPT_recovery;
  _fec->DWORD1.E               = FEC_E_1;
  _fec->DWORD2.TS_recovery     = pTS_recovery;
  _fec->DWORD3.SNBase_ext_bits = 0;
    //((pSNBase & 0xFFFF0000) >> 16) & 0x000000FF;
  _fec->DWORD3.NA              = pD == COL ? pD_rows : pL_cols;
  _fec->DWORD3.Offset          = pD == COL ? pL_cols : 1;
  _fec->DWORD3.index           = FEC_INDEX_XOR;
  _fec->DWORD3.type            = XOR;
  _fec->DWORD3.D               = pD;
  _fec->DWORD3.X               = FEC_X_0;
  _fec->resXor                 = 0;

  if (pLength_recovery == 0) return _fec;

  _fec->resXor = malloc (pLength_recovery);
  if (_fec->resXor == NULL) // Allocation ratée ?
  {
    free (_fec);
    return NULL;
  }

  if (pResXor == 0)
  {
    memset (_fec->resXor, 0, pLength_recovery);
  }
  else
  {
    // TODO Attention : trop grande confiance en le pResXor donné en paramètre
    void*     ok = memcpy (_fec->resXor, pResXor, pLength_recovery);
    if (ok == NULL) // Allocation ratée ?
    {
      sPaquetFec_Release(_fec);
      return NULL;
    }
  }

  return _fec;
}

// Libère la mémoire allouée par un paquet de FEC ------------------------------
void sPaquetFec_Release
  (sPaquetFec* pFec) //: Paquet à vider
{
  if (pFec == NULL)
    return;

  if (pFec->resXor != NULL)
  {
    free (pFec->resXor);
  }
  free (pFec);
}

// Retourne un fecNx à partir d'un fecNo ---------------------------------------
//> Le numéro de séquence fecNx équivalent
inline sFecNx sFecNo_to_sFecNx
  (sFecNo pFecNo) //: Le fecNo à convertir
{
  return (sFecNx) {pFecNo, false};
}


//------------------------------------------------------------------------------
//              CROSS FEC
//------------------------------------------------------------------------------

// Constantes privées ==========================================================

const sCrossFec INIT_CROSS_FEC= //. Valeur par défaut d'un cross (à FEC_NX_NULL)
  {{0, true}, {0, true}};

// Fonctions publiques =========================================================

// Création d'un nouveau cross                                           -------
// Remarque : ne pas oublier de faire le ménage avec sCrossFec_Release ! -------
//> Pointeur sur le nouveau cross ou 0 si problème
sCrossFec* sCrossFec_New()
{
  sCrossFec* _cross = malloc (sizeof (sCrossFec));
  if (_cross == NULL)// Allocation ratée ?
    return NULL;

  *_cross = INIT_CROSS_FEC; // Initialise le contenu du cross

  return _cross;
}

// Libère la mémoire allouée par un cross --------------------------------------
void sCrossFec_Release
  (sCrossFec* pCross) //: Cross à vider
{
  if (pCross == NULL)
    return;
  free     (pCross);
}


//------------------------------------------------------------------------------
//              WAIT FEC
//------------------------------------------------------------------------------
// Fonctions publiques =========================================================

// Création d'un nouveau wait                                           --------
// Remarque : ne pas oublier de faire le ménage avec sWaitFec_Release ! --------
//> Pointeur sur le nouveau wait ou 0 si problème
sWaitFec* sWaitFec_New
  (bool pInitParams) //: Faut-il initialiser les paramètres (à 0) ?
{
  return pInitParams ? calloc (sizeof (sWaitFec), 1) :
                       malloc (sizeof (sWaitFec));
}

// Création d'un nouveau wait à partir d'un paquet de FEC               --------
// Remarque : ne pas oublier de faire le ménage avec sWaitFec_Release ! --------
//> Pointeur sur le nouveau wait ou 0 si problème
sWaitFec* sWaitFec_Forge
  (const sPaquetFec* pFec) //: Le paquet d'où prendre les paramètres
{
  if (pFec == NULL)
    return NULL;

  sWaitFec* _wait = sWaitFec_New (false);
  if (_wait == NULL) // Allocation ratée ?
    return NULL;

  _wait->fecNo           = pFec->fecNo;
  _wait->Length_recovery = pFec->DWORD0.Length_recovery;
  _wait->PT_recovery     = pFec->DWORD1.PT_recovery;
  _wait->TS_recovery     = pFec->DWORD2.TS_recovery;
  _wait->SNBase          = pFec->DWORD0.SNBase_low_bits/*+
                           pFec->DWORD3.SNBase_ext_bits*256*256*/;
  _wait->Offset          = pFec->DWORD3.Offset;
  _wait->NA              = pFec->DWORD3.NA;
  _wait->number          = 0;
  _wait->missing         = sChampBits_New();
  _wait->D               = pFec->DWORD3.D;
  _wait->resXor          = pFec->resXor;

  if (_wait->resXor == 0)
    return _wait;

  _wait->resXor = malloc (_wait->Length_recovery);
  if (_wait->resXor == NULL) // Allocation ratée ?
  {
    sWaitFec_Release (_wait);
    return NULL;
  }

  // Attention : trop grande confiance en le p...Fec.resXor donné en paramètre
  void*     ok = memcpy (_wait->resXor, pFec->resXor, _wait->Length_recovery);
  if (ok == false) // Copie ratée ?
  {
    sWaitFec_Release (_wait);
    return NULL;
  }

  return _wait;
}

// Libère la mémoire allouée par un wait ---------------------------------------
void sWaitFec_Release
  (sWaitFec* pWait) //: Wait à vider
{
  if (pWait == NULL)
    return;

  if (pWait->resXor)
  {
    free (pWait->resXor);
  }

  free (pWait);
}

// Calcule mediaNx d'un des paquets média manquant à partir du wait ------------
//> Valeur de mediaNx calculée ou MEDIA_NX_NULL si problème (pNo hors-limites)
sMediaNx sWaitFec_GetManque
  (const sWaitFec* pWait, //: Wait qui permet de calculer médiaNx
         uint8_t   pNo)   //: Numéro de manque (1=le médiaNo 1er paq. manquant)
{
  if (pWait == NULL)
    return MEDIA_NX_NULL;
  if ( !(pNo > 0 && pNo <= pWait->number) )
    return MEDIA_NX_NULL;

  signed j = sChampBits_GetOne (&pWait->missing, pNo, LSB_FIRST);
  if (j == -1)// J ne doit pas indiquer un problème
    return MEDIA_NX_NULL;

  return sMediaNo_to_sMediaNx (pWait->SNBase + pWait->Offset * j);
}

// Met à jour wait quand un paquet média est signalé manquant ou retrouvé ------
bool sWaitFec_SetManque
  (sWaitFec* pWait,   //: Wait à modifier
   uint8_t   pNo,     //: Numéro du bit à changer
   bool      pValeur) //: Valeur à affecter au bit
{
  if (pWait == NULL)
    return false;
  // TODO Gérer (Set à la même valeur pour le bit = bug structure)
  if (sChampBits_GetBit (&pWait->missing, pNo) == pValeur)
    return false;

  pWait->number = pValeur ? pWait->number + 1 : pWait->number - 1;

  sChampBits_SetBit (&pWait->missing, pNo, pValeur);

  return true;
}

// Retrouve j à partir de médiaNo : médiaNo = SNBase + j * Offset --------------
//> Valeur de j ou MEDIA_NX_NULL si j hors bornes du wait (ce qui est faux)
sMediaNx sWaitFec_ComputeJ
  (const sWaitFec* pWait,    //: Wait qui permet de calculer j
         sMediaNo  pMediaNo) //: MédiaNo à déduire à partir des params de wait
{
  if (pWait == NULL)
    return MEDIA_NX_NULL;
  if (pWait->Offset <= 0)// Doit être supérieur à 0 !
    return MEDIA_NX_NULL;

  sMediaNo j;
  sMediaNo _mediaTest = pWait->SNBase;

  // TODO Pas optimale mais fonctionne en modulaire ;-)
  for (j = 0; j < pWait->NA; j++)
  {
    if (pMediaNo == _mediaTest)
      return sMediaNo_to_sMediaNx (j);

    _mediaTest += pWait->Offset;
  }

  return MEDIA_NX_NULL;
}


//------------------------------------------------------------------------------
//              BUFFER FEC
//------------------------------------------------------------------------------

// Fonctions privées ===========================================================

// Libère un noeud du type cross -----------------------------------------------
void ReleaseCrossFunc
  (uint32_t pKey,   //: Clé (médiaNo) du noeud à supprimer
   void*    pValue) //: Valeur (cross) du noeud à supprimer
{
  if (pValue) sCrossFec_Release (pValue);
}

// Libère un noeud du type wait ------------------------------------------------
void ReleaseWaitFunc
  (uint32_t pKey,   //: Clé (fecNo) du noeud à supprimer
   void*    pValue) //: Valeur (wait) du noeud à supprimer
{
  if (pValue) sWaitFec_Release (pValue);
}

// Fonctions publiques =========================================================

// Créé un buffer de FEC -------------------------------------------------------
// Remarque : ne pas oublier de faire le ménage avec sBufferFec_Release ! ------
//> Nouveau buffer de FEC
sBufferFec_t *sBufferFec_New()
{
  sBufferFec_t *_buffer = malloc (sizeof (sBufferFec_t));
     if (_buffer == NULL)
        return NULL;

  _buffer->cross     = sRbTree_New (ReleaseCrossFunc);
  _buffer->wait[COL] = sRbTree_New (ReleaseWaitFunc);
  _buffer->wait[ROW] = sRbTree_New (ReleaseWaitFunc);

  return _buffer;
}

// Libère la mémoire allouée par un buffer de FEC ------------------------------
void sBufferFec_Release
  (sBufferFec_t* pBuffer) //: Buffer à vider
{
  if (pBuffer == NULL)
    return;

  sRbTree_Release (&pBuffer->cross);
  sRbTree_Release (&pBuffer->wait[COL]);
  sRbTree_Release (&pBuffer->wait[ROW]);
}

// Insère un nouveau cross dans le buffer de FEC -------------------------------
//> Status de l'opération / ajout réussi ?
bool sBufferFec_AddCrossByReference
  (sBufferFec_t* pBuffer,  //: Buffer à modifier
   sMediaNo    pMediaNo, //: Lier le cross à ce médiaNo
   sCrossFec*  pCross,   //: Cross à ajouter
   bool pOver) //: Faut-il écraser un noeud qui porterait déjà key=médiaNo ?
{
  if (pBuffer == NULL || pCross == NULL)
    return false;

  // Enregistre le cross dans bufferFec.cross
  return sRbTree_AddByReference
    (&pBuffer->cross, pMediaNo, (void*)pCross, pOver) != 0;
}

// Retrouve un cross lié au médiaNo donné en paramètre -------------------------
//> Pointeur sur le cross lié à key=médiaNo ou 0 si inexistant
sCrossFec* sBufferFec_FindCross
  (const sBufferFec_t* pBuffer,  //: Buffer à traiter
         sMediaNo    pMediaNo) //: Paramètre de recherche
{
  if  (pBuffer == NULL)
    return NULL;

  return sRbTree_Lookup (&pBuffer->cross, pMediaNo);
}

// Supprime un cross -----------------------------------------------------------
//> Status de l'opération
bool sBufferFec_DeleteCross
  (sBufferFec_t* pBuffer,  //: Buffer à modifier
   sMediaNo    pMediaNo) //: MédiaNo du cross à supprimer
{
  if (pBuffer == NULL)
    return false;

  return sRbTree_Delete (&pBuffer->cross, pMediaNo);
}

// Insère un nouveau wait ------------------------------------------------------
//> Status de l'opération
bool sBufferFec_AddWaitByReference
  (sBufferFec_t* pBuffer, //: Buffer à modifier
   sWaitFec  * pWait,   //: Wait à ajouter
   bool pOver) //: Faut-il écraser un noeud qui porterait déjà key=pWait.fecNo ?
{
  if (pBuffer == NULL || pWait == NULL)
    return false;

  // Enregistre le wait dans bufferFec.wait[D]

  return sRbTree_AddByReference (&pBuffer->wait[pWait->D],
                                 pWait->fecNo, (void*)pWait, pOver) != 0;
}

// Retrouve un wait lié à une direction et un fecNo donné en paramètre ---------
//> Pointeur sur le wait[direction=D] lié à key=fecNo ou 0 si inexistant
sWaitFec* sBufferFec_FindWait
  (const sBufferFec_t* pBuffer, //: Buffer à traiter
         eFecD       pD,      //: Faut-il chercher dans wait[COL] ou wait[ROW] ?
         sFecNo      pFecNo)  //: Paramètre de recherche
{
  if (pBuffer == NULL)
    return NULL;

  return sRbTree_Lookup (&pBuffer->wait[pD], pFecNo);
}

// Supprimer un wait du buffer de FEC ------------------------------------------
//> Status de l'opération / suppression réussie ?
bool sBufferFec_DeleteWait
  (sBufferFec_t* pBuffer, //: Buffer à modifier
   eFecD       pD,      //: Faut-il opérer dans wait[COL] ou wait[ROW] ?
   sFecNo      pFecNo)  //: fecNo du wait à supprimer
{
  if (pBuffer == NULL)
    return false;

  return sRbTree_Delete (&pBuffer->wait[pD], pFecNo);
}

// Fait le ménage en supprimant toutes traces concernant un paquet de FEC   ----
// particulier. Ce nettoyage est nécessaire au cas où certains paquets de   ----
// FEC seraient devenus obsolètes. Par exemple il serait inutile de         ----
// récupérer des paquets média déjà " sautés " (manquants à la lecture) par ----
// le player...                                                             ----
//> Status de l'opération / nettoyage réussi ?
bool sBufferFec_DeleteCrossAndWait
  (sBufferFec_t* pBuffer, //: Buffer à modifier
   eFecD       pD,      //: Faut-il opérer dans wait[COL] ou wait[ROW] ?
   sFecNo      pFecNo)  //: fecNo du wait (cross retrouvés depuis le wait)
{
  if (pBuffer == NULL)
    return false;

  sWaitFec* _wait = sBufferFec_FindWait (pBuffer, pD, pFecNo);
  if (_wait == NULL)
    return false; // Wait introuvable ?

  uint8_t j;

  for (j = 0; j < _wait->number; j++)
  {
    sMediaNx _mediaUpd = sWaitFec_GetManque (_wait, j + 1);
    if ( _mediaUpd.null == true)
      return false;

    sCrossFec* _crossUpd = sBufferFec_FindCross (pBuffer, _mediaUpd.v);
    if (!_crossUpd) continue;

    if (pD == COL)
      _crossUpd->colNx = FEC_NX_NULL;
    else
      _crossUpd->rowNx = FEC_NX_NULL;

    // Entrée dans cross devenue inutile
    if (_crossUpd->colNx.null && _crossUpd->rowNx.null)
    {
      bool ok = sBufferFec_DeleteCross (pBuffer, _mediaUpd.v);
      if (!ok)
        return false;
    }
  }

  return sBufferFec_DeleteWait (pBuffer, pD, pFecNo);
}

// Initalise la boucle foreach like sur le buffer de FEC (cross) ---------------
//> Status de l'opération / Est-ce que ForeachData est (un cross) valide ?
bool sBufferFec_InitForeachCross
  (sBufferFec_t* pBuffer,  //: Buffer à traiter
   bool        pReverse) //: Faut-il effectuer la boucle à " l'envers " ?
{
  if (pBuffer == NULL)
    return false;

  return sRbTree_InitForeach (&pBuffer->cross, pReverse);
}

// Continue la boucle foreach like sur le buffer de FEC (cross) ----------------
//> Status de l'opération / Est-ce que ForeachData est (un cross) valide ?
bool sBufferFec_NextForeachCross
  (sBufferFec_t* pBuffer) //: Buffer à traiter
{
  if (pBuffer == NULL)
    return false;

  return sRbTree_NextForeach (&pBuffer->cross);
}

// Retourne le médiaNo lié au cross (pointé) par la boucle foreach -------------
//> médiaNo lié au cross (pointé) par la boucle foreach ou 0 si inactif
sMediaNo sBufferFec_ForeachKeyCross
  (const sBufferFec_t* pBuffer) //: Buffer à traiter
{
  if (pBuffer == NULL)
    return 0;

  return sRbTree_ForeachKey (&pBuffer->cross);
}

// Retourne un pointeur sur le cross (pointé) par la boucle foreach ------------
//> Pointeur sur le cross (pointé) par la boucle foreach ou 0 si inactif
sCrossFec* sBufferFec_ForeachValueCross
  (const sBufferFec_t* pBuffer) //: Buffer à traiter
{
  if (pBuffer == NULL)
    return NULL;

  return sRbTree_ForeachValue (&pBuffer->cross);
}
