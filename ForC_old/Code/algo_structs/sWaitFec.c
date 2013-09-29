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

#include "../smpte.h"

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
  ASSERTpc (pFec, 0, cExNullPtr)

  sWaitFec* _wait = sWaitFec_New (false);
  IFNOT    (_wait, 0) // Allocation ratée ?

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

  if (_wait->resXor == 0) return _wait;

  _wait->resXor = malloc (_wait->Length_recovery);
  IFNOT_OP (_wait->resXor, sWaitFec_Release (_wait), 0) // Allocation ratée ?

  // Attention : trop grande confiance en le p...Fec.resXor donné en paramètre
  void*     ok = memcpy (_wait->resXor, pFec->resXor, _wait->Length_recovery);
  IFNOT_OP (ok, sWaitFec_Release (_wait), 0) // Copie ratée ?

  return _wait;
}

// Libère la mémoire allouée par un wait ---------------------------------------
void sWaitFec_Release
  (sWaitFec* pWait) //: Wait à vider
{
  ASSERTpc (pWait,, cExNullPtr)

  if (pWait->resXor)
  {
    free (pWait->resXor);
  }

  free (pWait);
}

// Affiche le contenu d'un wait ------------------------------------------------
void sWaitFec_Print
  (const sWaitFec* pWait) //: Wait à afficher
{
  ASSERTpc (pWait,, cExNullPtr)

  PRINT1 ("{fecNo=%u snb=%u off=%d na=%d nb=%u, missing=",
         pWait->fecNo, pWait->SNBase, pWait->Offset, pWait->NA, pWait->number)
  sChampBits_Print (&pWait->missing);
  PRINT1 ("d=%s} ", pWait->D == COL ? "COL" : "ROW")
}

// Calcule mediaNx d'un des paquets média manquant à partir du wait ------------
//> Valeur de mediaNx calculée ou MEDIA_NX_NULL si problème (pNo hors-limites)
sMediaNx sWaitFec_GetManque
  (const sWaitFec* pWait, //: Wait qui permet de calculer médiaNx
         uint8_t   pNo)   //: Numéro de manque (1=le médiaNo 1er paq. manquant)
{
  ASSERTpc (pWait, MEDIA_NX_NULL, cExNullPtr)
  ASSERTp  (pNo > 0 && pNo <= pWait->number, MEDIA_NX_NULL, cExWaitLimit, pNo)

  signed j = sChampBits_GetOne (&pWait->missing, pNo, LSB_FIRST);
  IFNOT (j != -1, MEDIA_NX_NULL) // J ne doit pas indiquer un problème

  return sMediaNo_to_sMediaNx (pWait->SNBase + pWait->Offset * j);
}

// Met à jour wait quand un paquet média est signalé manquant ou retrouvé ------
bool sWaitFec_SetManque
  (sWaitFec* pWait,   //: Wait à modifier
   uint8_t   pNo,     //: Numéro du bit à changer
   bool      pValeur) //: Valeur à affecter au bit
{
  // TODO Gérer (Set à la même valeur pour le bit = bug structure)
  ASSERTpc (sChampBits_GetBit (&pWait->missing, pNo) != pValeur, false,
            cExAlgorithm)

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
  ASSERTpc (pWait,             MEDIA_NX_NULL, cExNullPtr)
  IFNOT    (pWait->Offset > 0, MEDIA_NX_NULL) // Doit être supérieur à 0 !

  sMediaNo j;
  sMediaNo _mediaTest = pWait->SNBase;

  // TODO Pas optimale mais fonctionne en modulaire ;-)
  for (j = 0; j < pWait->NA; j++)
  {
    if (pMediaNo == _mediaTest) return sMediaNo_to_sMediaNx (j);

    _mediaTest += pWait->Offset;
  }

  return MEDIA_NX_NULL;
}
