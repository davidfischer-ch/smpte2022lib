
/******************************************************************************\
              ALGORITHME DE FEC SMPTE 2022-1 DAVID (OPTIMAL SELON MOI)

  Auteur    : David Fischer
  Intégration VLC : Rossier Jérémie (2011)
  Contact   : david.fischer.ch@gmail.com / david.fischer@hesge.ch
              jeremie.rossier@gmail.com

  Projet     : Implémentation SMPTE 2022 de VLC
  Date début : 02.05.2008
  Employeur  : Ecole d'Ingénieurs de Genève
               Laboratoire de Télécommunications
\******************************************************************************/

/* Copyright (c) 2009 David Fischer (david.fischer.ch@gmail.com)

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from:
  https://sourceforge.net/projects/smpte2022lib/
*/
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdarg.h>
#include <assert.h>
#include <errno.h>

#include <vlc_common.h>
#include <vlc_demux.h>

#include "smpte2022.h"
#include "smpte2022algorithm.h"
#include "rtp.h"

// Fonctions publiques =========================================================

// Création d'un nouveau David SMPTE                                       -----
// Remarque : ne pas oublier de faire le ménage avec sDavidSmpte_Release ! -----
//> Nouveau David SMPTE
sDavidSmpte_t *sDavidSmpte_New()
{

  sDavidSmpte_t *_david = malloc (sizeof (sDavidSmpte_t));
     if (_david == NULL)
        return NULL;

  _david->media                = sBufferMedia_New();
  _david->fec                  = sBufferFec_New();
  _david->recovered            = 0;
  _david->unrecoveredOnReading = 0;
  _david->nbArPaMedia          = 0;
  _david->nbArPaFec            = 0;
  _david->nbLePaMedia          = 0;
  _david->nbPePaMedia          = 0;
  _david->nbRePaMedia          = 0;
  _david->maxC                 = 0;
  _david->maxW                 = 0;
  _david->resized_matrix       = false;
  _david->resized_matrix_check = mdate ()+3000000; //Time to change matrix size 3sec

  return _david;
}

// Libère la mémoire allouée par l'algorithme optimisé -------------------------
void sDavidSmpte_Release
  (sDavidSmpte_t* pDavid) //: David SMPTE à vider
{
  if (pDavid == NULL)
     return;

  sBufferMedia_Release (pDavid->media);
  sBufferFec_Release   (pDavid->fec);
}

/*******************************************************************************
*                    GÈRE L'ARRIVÉE DE PAQUETS (MEDIA ou FEC)                  *
*******************************************************************************/

// Un paquet média vient d'arriver (ou d'être récupéré), tente de trouver    ---
// une entrée correspondante dans le buffer de FEC (via médiaNo). Si celle-  ---
// ci existe alors il faut faire appel au mécanisme de récuperation pour     ---
// mettre à jour buffer de FEC et peut-être débloquer la cascade de          ---
// récupération à partir de paquets FEC qui étaient jusque là en attente     ---
bool sDavidSmpte_ArriveePaquetMedia
  (demux_t *demux,
   sDavidSmpte_t * pDavid, //: David SMPTE à mettre à jour
   sPaquetMedia* pMedia) //: Paquet média arrivant (du réseau)
{
  if (pDavid == NULL || pMedia == NULL )
    return false;

  bool ok = sBufferMedia_AddByReference
              (pDavid->media, pMedia, true);
  if (! ok)
    return false;

//  msg_Dbg (demux, "SMPTE2022 : %"PRIu16" at %"PRId64" ", pMedia->mediaNo, mdate ());

  // Le paquet média est signalé comme perdu dans FEC : simuler la récup. !
  sCrossFec* _cross = sBufferFec_FindCross (pDavid->fec, pMedia->mediaNo);

  if (_cross != 0)
    //le paquet media est cité dans bufferFec.cross
    sDavidSmpte_RecupPaquetMedia (demux, pDavid, pMedia->mediaNo, _cross, 0);
  return true;
}

// Un paquet de Media vient d'arriver avec entête RTP,                       ---
//translation entre la structure RTP et la structure smpte20022              ---
bool sDavidSmpte_ArriveePaquetMedia_Convert
  (demux_t *demux,
   sDavidSmpte_t * pDavid, //: David SMPTE à mettre à jour
   block_t *block) //: Paquet média arrivant (du réseau) avec entête RTP à translater
{
  if (pDavid == NULL || block == NULL )
    return false;

  assert (block->i_buffer >= 12);
  uint8_t  _PayloadType = (block->p_buffer[1] & 0x7F);
  uint16_t _Seq =  GetWBE (block->p_buffer + 2);    
  uint32_t _Timestamp = GetDWBE (block->p_buffer + 4);

  //Check bit M in RTP header -> must be 0
  if ( (block->p_buffer[1] & 0x80) == 1)
    return false;
  size_t _PayloadSize = block->i_buffer - 12; //remove RTP header
  uint8_t *_Payload = block->p_buffer + 12; //pointer of RTP payload data

  //Forge the Media structure for smpte2022, payload coming from RTP buffer, don't malloc
  sPaquetMedia* _pMedia = sPaquetMedia_Forge(_Seq, _Timestamp, _PayloadType, 0, 0);
  _pMedia->payloadSize = _PayloadSize;
  _pMedia->payload = _Payload;
  _pMedia->payloadWithHeader = block->p_buffer;

  return sDavidSmpte_ArriveePaquetMedia(demux, pDavid, _pMedia);
}

// Un paquet de FEC vient d'arriver, liste les paquets média manquants que   ---
// le nouveau paquet de FEC est capable de récupérer.                        ---
// Dès lors 3 cas de figures peuvent arriver au paquet de FEC :              ---
// [1] Inutile (aucun des paquets média protégés ne manque) > jeté           ---
// [2] Récupère le seul paquet média manquant > opération exécutée puis jeté ---
// [3] Bloqué car >1 paquets média manquent> stocké pour cascade future      ---
void sDavidSmpte_ArriveePaquetFec
  (demux_t *demux,
   sDavidSmpte_t* pDavid, //: David SMPTE à mettre à jour
   sPaquetFec * pFec)   //: Paquet de FEC arrivant (du réseau)
{
 if (pDavid == NULL || pFec == NULL )
    return;

  // Lecture des champs du paquet SMPTE 2022-1 FEC
  if (pFec->DWORD1.Mask  != FEC_MASK_0)    return; // doit être 0
  if (pFec->DWORD3.X     != FEC_X_0)       return; // doit être 0
  if (pFec->DWORD3.type  != XOR)           return; // doit être XOR
  if (pFec->DWORD3.index != FEC_INDEX_XOR) return; // doit être 0

  sWaitFec* _wait = sWaitFec_Forge (pFec);
  if (_wait == NULL)
    return;

  sPaquetFec_Release (pFec);

  sCrossFec* _crossLast = 0;
  sMediaNo   _mediaLast = 0;
  sMediaNo   _mediaTest = _wait->SNBase;
  sMediaNo   _mediaMax  = _wait->SNBase + _wait->NA * _wait->Offset;

  // Paquet média protégés : médiaNo = SNBase + j*offset, avec j entre [0;NA[
  for (; _mediaTest != _mediaMax; _mediaTest += _wait->Offset)
  {
    if (sBufferMedia_Find (pDavid->media, _mediaTest) == NULL)
    {
      _crossLast =
        sDavidSmpte_PerduPaquetMedia (demux, pDavid, _mediaLast = _mediaTest, _wait);
    }
  }

  // [1] Aucune entrée enregistrée : paquet de FEC inutile à conserver
  if (_wait->number == 0)
  {
    sWaitFec_Release (_wait);
    goto __fin_chrono;
  }

  // Enregistre le paquet de FEC dans bufferFec.wait[D]

  bool    ok = sBufferFec_AddWaitByReference (pDavid->fec, _wait, false);
  if (ok == false)
    return;

  if (pDavid->fec->wait[_wait->D].count > pDavid->maxW)
  {
    pDavid->maxW = pDavid->fec->wait[_wait->D].count;
  }

  // [2] Qu'un seul paquet média manquant : récupération possible
  if (_wait->number == 1)
    sDavidSmpte_RecupPaquetMedia (demux, pDavid, _mediaLast, _crossLast, _wait);

  // [3] Récupération impossible pour l'instant, conserver le paquet de fec

__fin_chrono:
  pDavid->nbArPaFec++;
}

// Un paquet de FEC vient d'arriver avec entête RTP,                         ---
//translation entre la structure RTP et la structure smpte20022              ---
void sDavidSmpte_ArriveePaquetFec_Convert
  (demux_t *demux, 
   sDavidSmpte_t* pDavid, //: David SMPTE à mettre à jour
   block_t *block) //: Paquet média arrivant (du réseau) avec entête RTP à translater
{
  assert (block->i_buffer >= 28); //RTP header and SMPTE2022 header
  //Check bit M in RTP header -> must be 0
  if ( (block->p_buffer[1] & 0x80) == 1)
    return;
  //RTP header
  uint16_t _Seq =  GetWBE (block->p_buffer + 2);
  //SMPTE2022 header
  uint16_t _SNBase = GetWBE (block->p_buffer + 12);
  uint16_t _LengthRecovery =  GetWBE (block->p_buffer + 14);
  if (_LengthRecovery == 0) //Length Recovery should be used, but sometime not -> use payload size
    _LengthRecovery = (block->i_buffer) - 28; // RTP_Header 12 + SMPTE2022_Header 16

  if  ((block->p_buffer[16] & 0x80) == 0 ) //E bit must be 1
    return;
  uint8_t  _PTRecovery = ( block->p_buffer[16] & 0x7F);

  uint32_t _TSRecovery = GetDWBE (block->p_buffer + 20);

  if  ((block->p_buffer[24] & 0xBF) != 0 ) //N + Type + index bits must be 0
    return;

  uint8_t _Cols, _Rows;
  eFecD _Direction;
  if  ((block->p_buffer[24] & 0x40) != 0 ) //D bit : '0' column, '1' row
  {
    _Direction = ROW;
    _Cols = block->p_buffer[26];// NA bits
    _Rows = 1;// 1 == Offset
    if (!(pDavid->resized_matrix))
    {
      if (pDavid->resized_matrix_check - mdate() <= 0)
        {
          pDavid->resized_matrix = true;
          demux->p_sys->smpte2022_matrix_size = (_Cols)+10;
          msg_Dbg(demux, "SMPTE2022 Matrix change size %"PRIu16" ",demux->p_sys->smpte2022_matrix_size);
        }
    }
  }
  else
  {
    _Direction = COL;
    _Cols = block->p_buffer[25];//Offset bits
    _Rows = block->p_buffer[26];//NA bits
    if (!(pDavid->resized_matrix))
    {
      pDavid->resized_matrix = true;
      demux->p_sys->smpte2022_matrix_size = (_Cols * _Rows)+10;
      msg_Dbg(demux, "SMPTE2022 Matrix change size %"PRIu16" ",demux->p_sys->smpte2022_matrix_size);
    }
  }

  uint8_t* _ResXor = block->p_buffer + 28; //pointer of SMPTE2022 payload

  /*smpte2022 fec paquet creation */
  sPaquetFec* _ArrivedFec = sPaquetFec_Forge (_Seq, _LengthRecovery, _SNBase,
      _PTRecovery, _TSRecovery, _Cols, _Rows, _Direction, _ResXor);
  if (_ArrivedFec == NULL)
    return;

/*  //afficher des champs pour controler
  if (_ArrivedFec->fecNo > 19210 && _ArrivedFec->fecNo < 19220)
  msg_Dbg (demux, "SMPTE2022 : no %"PRIu16", Length %"PRIu16", Length_shoube %"PRIu16"",
           _ArrivedFec->fecNo, _ArrivedFec->DWORD0.Length_recovery,_LengthRecovery);
*/
  sDavidSmpte_ArriveePaquetFec (demux, pDavid, _ArrivedFec);
}

// Imite la lecture du buffer média et en profite pour nettoyer les buffers ----
//> Est-ce qu'une lecture a eu lieu ?
bool sDavidSmpte_LecturePaquetMedia
  (demux_t *demux,
   sDavidSmpte_t* pDavid,      //: David SMPTE à mettre à jour
   sMediaNo     _sMediaNo) //: paquets média à supprimer dans le buffer
{
  if (pDavid == NULL)
    return false;

  // Supprime l'élément du buffer
  bool ok = sRbTree_Delete (&pDavid->media->rbtree, _sMediaNo);
  if (! ok )
    return false;

  // Nettoye le buffer de FEC
  sCrossFec* _cross = sBufferFec_FindCross (pDavid->fec, _sMediaNo);

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

      sBufferFec_DeleteCrossAndWait (pDavid->fec, COL, _colNx.v);
    }

    if (!_rowNx.null)
    {
      // Utilise wait pour retrouver les entrées dans cross qui doivent
      // êtres supprimées (nettoyage)

      sBufferFec_DeleteCrossAndWait (pDavid->fec, ROW, _rowNx.v);
    }
  }

  pDavid->nbLePaMedia++;
  if (!ok) pDavid->unrecoveredOnReading++;

  return true;
}

/*******************************************************************************
*                  COEUR DE L'ALGORITHME DE FEC SMPTE 2022-1                     *
*******************************************************************************/

// Enregistre l'info comme quoi un paquet média manque et le lie au paquet   ---
// FEC ayant remarqué que le dit paquet média était manquant !               ---
// Attention : n'enregistre pas dans bufferFec.wait[D] (le fait plus tard)   ---
sCrossFec* sDavidSmpte_PerduPaquetMedia
  (demux_t *demux,
   sDavidSmpte_t* pDavid,   //: David SMPTE à mettre à jour
   sMediaNo     pMediaNo, //: MédiaNo du paquet média manquant
   sWaitFec*    pWait)    //: Wait (FEC) ayant remarqué la perte
{
  if (pDavid == NULL || pWait == NULL)
    return NULL;
//  msg_Dbg (demux, "SMPTE2022 lostPMedia : %"PRIu8" at %"PRId64" ", pMediaNo, mdate ());

  // ajoute la nouvelle entrée [lie paquet média manquant <-> paquet de FEC]

  // Recherche si l'entrée est déjà existante ...
  sCrossFec* _cross = sBufferFec_FindCross (pDavid->fec, pMediaNo);

  if (_cross == 0)
  {
    _cross = sCrossFec_New();
    if (_cross == NULL)
      return NULL;

    bool ok =
      sBufferFec_AddCrossByReference (pDavid->fec, pMediaNo, _cross, false);
    if (!ok)
      return NULL;

    if (pDavid->fec->cross.count > pDavid->maxC)
    {
      pDavid->maxC = pDavid->fec->cross.count;
    }
  }

  // Met à jour l'élément dans pBufferFec.cross

  if (pWait->D == COL)
  {
    if (!_cross->colNx.null)
      return NULL;
    _cross->colNx = sFecNo_to_sFecNx (pWait->fecNo);
  }
  else
  {
    if (!_cross->rowNx.null)
      return NULL;
    _cross->rowNx = sFecNo_to_sFecNx (pWait->fecNo);
  }

  sMediaNx j = sWaitFec_ComputeJ (pWait, pMediaNo);
  if (j.null)
    return NULL;

  // Enregistre que le paquet média médiaNo = (no_bit_dans_tab_manque) manque
  bool     ok = sWaitFec_SetManque (pWait, j.v, true);
  if (!ok)
    return NULL;

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
  (demux_t *demux,
   sDavidSmpte_t* pDavid,   //: David SMPTE à mettre à jour
   sMediaNo     pMediaNo, //: médiaNo du paquet média à récupérer (ou récupéré)
   sCrossFec*   pCross,   //: Cross du paquet média à récupérer (ou récupéré)
   sWaitFec*    pWait)    //: Wait (FEC) ayant remarqué la perte (si par FEC)
{
  if (pDavid == NULL || pCross == NULL)
    return;

  bool _parFec = pWait != 0; // [2 ou 3]
  unsigned no;

  // [1 ou 2 ou 3] Lecture des données du cross et suppression de ce dernier

  sFecNx _cascadeFecNx[2];
  _cascadeFecNx[COL] = pCross->colNx;
  _cascadeFecNx[ROW] = pCross->rowNx;

  bool ok = sBufferFec_DeleteCross (pDavid->fec, pMediaNo);
  if (!ok)
    return;

  // [2 ou 3] Récupère le paquet média
  //          Supprime les trace du paquet de FEC devenu inutile

  if (_parFec)
  {
    if (pWait->number != 1) //must only 1 Media paquet missing
      return;
    if (_cascadeFecNx[pWait->D].null)
      return;
    if (pWait->fecNo != _cascadeFecNx[pWait->D].v )
      return;

    // Etapes de la récupération (2 étapes) :
    // > payloadRecup = paquetFec.resXor

    sPaquetMedia* _recup = sPaquetMedia_Forge
        (pMediaNo, pWait->TS_recovery,     pWait->PT_recovery,
                   pWait->Length_recovery, pWait->resXor);
    if (_recup == NULL)
      return;

    // > payloadRecup ^= (tous les paquetMedia liés au paquetFec)
    sMediaNo _mediaNo  = pWait->SNBase;
    sMediaNo _mediaMax = pWait->SNBase + pWait->NA * pWait->Offset;

    // Paquet média protégés : médiaNo = SNBase + j*offset, avec j entre [0;NA[
    for (; _mediaNo != _mediaMax; _mediaNo += pWait->Offset)
    {
      if (_mediaNo == pMediaNo) continue;

      sPaquetMedia* _ami = sBufferMedia_Find (pDavid->media, _mediaNo);

      _recup->timeStamp   ^= _ami->timeStamp;
      _recup->payloadType ^= _ami->payloadType;

      unsigned _size = (_recup->payloadSize <= _ami->payloadSize ? _recup->payloadSize : _ami->payloadSize);
      for (no = 0; no < _size; no++)
      {
        // !!!! ATTENTION calcul payload; timestamp + payloadtype out of the loop
        // paquet media sans padding ! stop si ->end paquet media
        _recup->payload[no] ^= _ami->payload[no];
      }
    }

//  msg_Dbg (demux, "SMPTE2022 recup media: %"PRIu16", ts %"PRIu32" ", _recup->mediaNo, _recup->timeStamp);
    //TODO build media paquet with VLC struct !!
    block_t* _NewRTP_Media = block_Alloc (_recup->payloadSize+12); //Payload + RTP header (12)
    if (_NewRTP_Media == NULL)
    {
        msg_Dbg (demux,"probl smpte2022 block alloc");
        return;
    }

    //find a similar RTP Paquet to forge the RTP header
    sPaquetMedia* _ami;
    _mediaNo  = pWait->SNBase;
    for (; _mediaNo != _mediaMax; _mediaNo += pWait->Offset)
    {
      if (_mediaNo == pMediaNo) continue;
      _ami = sBufferMedia_Find (pDavid->media, _mediaNo);
      break;
    }

    if (_ami == NULL)
    {
      block_Release (_NewRTP_Media);
      return;
    }
    //msg_Dbg (demux, "SMPTE2022 recup media header from %"PRIu8" ", _ami->mediaNo);

    //loop RTP header : 12 bytes
    for (no = 0; no < 12; no++)
    {
      if (no > 1 && no < 8 ) continue; //don't copy PT, Seq and TS
      _NewRTP_Media->p_buffer[no]= _ami->payloadWithHeader[no];
    }
    //M flag + PT
    _NewRTP_Media->p_buffer[1] = (_NewRTP_Media->p_buffer[1] & 0x80) + (_recup->payloadType & 0x7F);
    // Seq
    _NewRTP_Media->p_buffer[2] = (uint8_t) (_recup->mediaNo >> 8); //high part
    _NewRTP_Media->p_buffer[3] = (uint8_t) (_recup->mediaNo & 0x00ff); //down part
    //TS
    _NewRTP_Media->p_buffer[4] = (uint8_t) (_recup->timeStamp >>24);
    _NewRTP_Media->p_buffer[5] = (uint8_t) ((_recup->timeStamp >>16) & 0xff);
    _NewRTP_Media->p_buffer[6] = (uint8_t) ((_recup->timeStamp >>8) & 0xff);
    _NewRTP_Media->p_buffer[7] = (uint8_t) (_recup->timeStamp & 0xff);
    //msg_Dbg (demux, "SMPTE2022 recup media header done %"PRIu8" ", GetWBE (_NewRTP_Media->p_buffer + 2)  );

    //RTP payload
    for (no = 0; no < _recup->payloadSize; no++)
      _NewRTP_Media->p_buffer[no+12] = _recup->payload[no]; //ERREUR SEGMENTATION Done

    //free _recup payload; associate newRTPMedia payload to _recup
//    if (_recup->payload != NULL)
//      free (_recup->payload);
    _recup->payload = (_NewRTP_Media->p_buffer + 12); //TODO CHECK
    _recup->payloadWithHeader = _NewRTP_Media->p_buffer;

    //put RTP Packet into RTP queue
    rtp_smpte2022_media_queue(demux,_NewRTP_Media);


    bool ok = sBufferMedia_AddByReference
                (pDavid->media, _recup, true);
    if (!ok)
      return;

    pDavid->recovered++;

    _cascadeFecNx[pWait->D] = FEC_NX_NULL;

    ok = sBufferFec_DeleteWait (pDavid->fec, pWait->D, pWait->fecNo);
    if (!ok)
      return;
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
      sBufferFec_FindWait (pDavid->fec, no, _cascadeFecNx[no].v);
    if (_cascadeWait[no] == NULL)
      return;

    sMediaNx j = sWaitFec_ComputeJ (_cascadeWait[no], pMediaNo);
    if (j.null)
      return;

    // Ne retire que le paquet média médiaNo = (no_bit_dans_tab_manque) manque
    sWaitFec_SetManque (_cascadeWait[no], j.v, false);
  }

  for (no = 0; no < 2; no++)
  {
    if (_cascadeFecNx[no].null) continue;

    if (_cascadeWait[no]->number == 1)
    {
      // Cascade !
      sMediaNx _cascadeMediaNx = sWaitFec_GetManque (_cascadeWait[no], 1);
      if (_cascadeMediaNx.null)
        return;

      sCrossFec* _cascadeCross =
        sBufferFec_FindCross (pDavid->fec, _cascadeMediaNx.v);
      if (_cascadeCross == NULL)
        return;

      sDavidSmpte_RecupPaquetMedia
        (demux, pDavid, _cascadeMediaNx.v, _cascadeCross, _cascadeWait[no]);
    }
  }

  pDavid->nbRePaMedia++;
}
