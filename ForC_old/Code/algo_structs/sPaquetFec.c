/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description : FEC packet struct
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

const sPaquetFec INIT_PAQUET_FEC = //. Valeur par défaut d'un paquet de FEC (0)
  {0, {0,0}, {0,0,0}, {0}, {0,0,0,0,0,0,0}, 0};

const char*  cBegFecString = "THIS_IS_A_FEC_PK"; //. Entête parsing -> fichier
const size_t cBegFecLength = 16;    //. Longueur de l'entête parsing
static char  cBegFecBuffer  [16+1]; //. Buffer lecture de l'entête parsing

// Fonctions publiques =========================================================

// Copie un paquet de FEC sans oublier de dupliquer (memcpy) le contenu   ------
// pointé par resXor !                                                    ------
// Remarque : ne pas oublier de faire le ménage avec sPaquetFec_Release ! ------
//> Pointeur sur le nouveau paquet de FEC ou 0 si problème
sPaquetFec* sPaquetFec_Copy
  (const sPaquetFec* pFec) //: Paquet à copier
{
  ASSERTpc (pFec, 0, cExNullPtr)

  sPaquetFec* _fec = malloc (sizeof (sPaquetFec));
  IFNOT      (_fec, 0) // Allocation ratée ?

  void*     ok = memcpy (_fec, pFec, sizeof (sPaquetFec));
  IFNOT_OP (ok, free (_fec), 0) // Copie ratée ?

  if (pFec->resXor == 0) return _fec;

  _fec->resXor = malloc (pFec->DWORD0.Length_recovery);
  IFNOT_OP (_fec->resXor, free (_fec), 0) // Allocation ratée

  // TODO Attention : trop grande confiance en le resXor donné en paramètre
  ok = memcpy (_fec->resXor, pFec->resXor, pFec->DWORD0.Length_recovery);
  IFNOT_OP (ok, sPaquetFec_Release (_fec), 0) // Copie ratée ?

  return _fec;
}

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
  IFNOT      (_fec, 0) // Allocation ratée ?

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
  IFNOT_OP (_fec->resXor, free (_fec), 0) // Allocation ratée ?

  if (pResXor == 0)
  {
    memset (_fec->resXor, 0, pLength_recovery);
  }
  else
  {
    // TODO Attention : trop grande confiance en le pResXor donné en paramètre
    void*     ok = memcpy (_fec->resXor, pResXor, pLength_recovery);
    IFNOT_OP (ok, sPaquetFec_Release (_fec), 0) // Allocation ratée ?
  }

  return _fec;
}

// Libère la mémoire allouée par un paquet de FEC ------------------------------
void sPaquetFec_Release
  (sPaquetFec* pFec) //: Paquet à vider
{
  ASSERTpc (pFec,, cExNullPtr)

  if (pFec->resXor)
  {
    free (pFec->resXor);
  }
  free (pFec);
}

// Affiche le contenu d'un paquet de FEC ---------------------------------------
void sPaquetFec_Print
  (const sPaquetFec* pFec) //: Paquet à afficher
{
  ASSERTpc (pFec,, cExNullPtr)

  PRINT1 ("{fecNo=%u, snb=%lu, off=%u, na=%u, d=%s} ",
          pFec->fecNo,
          (uint32_t)pFec->DWORD0.SNBase_low_bits +
          (uint32_t)pFec->DWORD3.SNBase_ext_bits*256*256,
          pFec->DWORD3.Offset,
          pFec->DWORD3.NA,
          pFec->DWORD3.D == COL ? "COL" : "ROW")
}

// Enregistre le contenu d'un paquet de FEC vers un fichier. Enregistre le -----
// paquet en entier (entête+parsing)                                       -----
//> Résultat de l'opération / enregistrement réussi ?
bool sPaquetFec_ToFile
  (const sPaquetFec* pFec,  //: Paquet à enregistrer
         FILE*       pFile) //: Fichier destination
{
  ASSERTpc (pFec,  false, cExNullPtr)
  ASSERTpc (pFile, false, cExNullPtr)

  bool ok =  (fwrite (cBegFecString, cBegFecLength,       1, pFile) == 1);
  ok = ok && (fwrite (pFec,          sizeof (sPaquetFec), 1, pFile) == 1);

  if (pFec->DWORD0.Length_recovery > 0)
  {
    ok = ok &&
      (fwrite (pFec->resXor, pFec->DWORD0.Length_recovery, 1, pFile) == 1);
  }

  return ok;
}

// Récupère le contenu d'un paquet de FEC depuis un fichier. Celui-ci doit -----
// avoir été enregistré avec l'entête+parsing !                            -----
//> Pointeur sur le paquet de FEC récupéré ou 0 si problème
sPaquetFec* sPaquetFec_FromFile
  (FILE* pFile) //: Fichier source
{
  ASSERTpc (pFile, 0, cExNullPtr)

  fpos_t pos;
  fgetpos (pFile, &pos);

  bool ok = fread (cBegFecBuffer, 1, cBegFecLength, pFile) == cBegFecLength;
  IFNOT_OP (ok, fsetpos (pFile, &pos), 0) // Lecture ratée ?

  cBegFecBuffer[cBegFecLength] = 0;

  // Le flux est bien préfixé ... alors ça doit être bon !
  ok = strcmp (cBegFecBuffer, cBegFecString) == 0;
  IFNOT_OP (ok, fsetpos (pFile, &pos), 0) // Comparaison réussie ?

  sPaquetFec* _fec = malloc (sizeof (sPaquetFec));
  IFNOT_OP   (_fec, fsetpos (pFile, &pos), 0) // Allocation ratée ?

  ok = fread (_fec, 1, sizeof (sPaquetFec), pFile) == sizeof (sPaquetFec);
  IFNOT_OP (ok, fsetpos (pFile, &pos); free (_fec), 0) // Lecture ratée ?

  _fec->resXor = 0;

  if (_fec->DWORD0.Length_recovery > 0)
  {
    _fec->resXor = malloc (_fec->DWORD0.Length_recovery);
    IFNOT_OP (_fec->resXor, fsetpos(pFile,&pos); free(_fec), 0) // Allocation ?

    ok = fread (_fec->resXor, 1, _fec->DWORD0.Length_recovery, pFile)
          == _fec->DWORD0.Length_recovery;

    IFNOT_OP (ok, fsetpos(pFile,&pos); sPaquetFec_Release(_fec), 0) // Lecture ?
  }

  return _fec;
}
