/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Bit-field : A big unisgned number
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

// Création d'un nouveau champ de bits -----------------------------------------
//> Nouveau champ de bits (initialisé à 0)
sChampBits sChampBits_New()
{
  sChampBits _champ;

  memset (&_champ, 0, sizeof (sChampBits));
  return   _champ;
}

// Affiche le contenu d'un champ de bits ---------------------------------------
void sChampBits_Print
  (const sChampBits* pChamp) //: Champ à afficher
{
  ASSERTpc (pChamp,, cExNullPtr)

  PRINT1 ("{ ")

  signed no;

  bool ok = false;

  for (no = CHAMP_NOMBRE_UNITE-1; no >= 0; no--)
  {
    if (pChamp->buffer[no] > 0) ok = true;

    if (ok) { PRINT1 ("%08lX ", pChamp->buffer[no]) }
  }

  PRINT1 ("} ")
}

// Lecture de l'état d'un des bits du champ de bits ----------------------------
//> Etat du bit
bool sChampBits_GetBit
  (const sChampBits* pChamp, //: Champ à traiter
   uint8_t           pNo)    //: Numéro (position) du bit à lire
{
  ASSERTpc (pChamp, false, cExNullPtr)

  uint8_t no  = pNo / CHAMP_TAILLE_UNITE;
  uint8_t bit = pNo - no * CHAMP_TAILLE_UNITE;

  return (pChamp->buffer[no] & (1 << bit)) > 0;
}

// Changer l'état d'un des bits du champ de bits -------------------------------
void sChampBits_SetBit
  (sChampBits* pChamp,  //: Champ à traiter
   uint8_t     pNo,     //: Numéro (position) du bit à changer
   bool        pValeur) //: Valeur à affecter
{
  ASSERTpc (pChamp,, cExNullPtr)

  uint8_t  no  = pNo      / CHAMP_TAILLE_UNITE;
  uint8_t  bit = pNo - no * CHAMP_TAILLE_UNITE;
  uint32_t val = pChamp->buffer[no];

  pChamp->buffer[no] = pValeur ? val |  (1 << bit) : // Or bit à 1
                                 val & ~(1 << bit);  // And not bit à 1
}

// Retourne le numéro du Xème bit à 1 (en effectuant la recherche du MSB au ----
// LSB et du LSB au MSB si pDirection=LSB_FIRST)                            ----
//> Position du bit à 1 ou -1 si non trouvé
signed sChampBits_GetOne
  (const sChampBits* pChamp,     //: Champ à traiter
   uint8_t           pNo,        //: Numéro (Xème) du bit à trouver
   eDir              pDirection) //: LSB_FIRST = LSB->MSB, MSB_FIRST = MSB->LSB
{
  ASSERTpc (pChamp,  0, cExNullPtr)
  ASSERTpc (pNo > 0, 0, cExChampBitNo)

  signed no, nb, n = 0;

  if (pDirection == LSB_FIRST)
  {
    for (no = 0; no < (signed)CHAMP_NOMBRE_UNITE; no++)
    {
      if (pChamp->buffer[no] == 0) continue;

      for (nb = 0; nb < (signed)CHAMP_TAILLE_UNITE; nb++)
      {
        if (pChamp->buffer[no] & (1 << nb))
        {
          if (++n == pNo) return no*CHAMP_TAILLE_UNITE+nb;
        }
      }
    }
  }
  else if (pDirection == MSB_FIRST)
  {
    for (no = (signed)(CHAMP_NOMBRE_UNITE-1); no >= 0; no--)
    {
      if (pChamp->buffer[no] == 0) continue;

      for (nb = (signed)(CHAMP_TAILLE_UNITE-1); nb >= 0; nb--)
      {
        if (pChamp->buffer[no] & (1 << nb))
        {
          if (++n == pNo) return no*CHAMP_TAILLE_UNITE+nb;
        }
      }
    }
  }

  return -1;
}

// Incrémente la valeur entière non signée représentée par le champ de bits ----
void sChampBits_Incremente
  (sChampBits* pChamp) //: Champ à traiter
{
  ASSERTpc (pChamp,, cExNullPtr)

  unsigned no;

  for (no = 0; no < CHAMP_NOMBRE_UNITE; no++)
  {
    pChamp->buffer[no]++;

    // Aucune retenue
    if (pChamp->buffer[no] > 0) break;
  }
}

// Compare les valeurs entières non signées représentées par les champs --------
//> Résultat: <0 si champ1 < champ2 / 0 si égalité / >0 si champ1 > champ2
signed sChampBits_Compare
  (const sChampBits* pChamp1, //: Champ à comparer
   const sChampBits* pChamp2) //: Champ à comparer
{
  ASSERTpc (pChamp1, 0, cExNullPtr)
  ASSERTpc (pChamp2, 0, cExNullPtr)

  signed no;

  for (no = CHAMP_NOMBRE_UNITE-1; no >= 0; no--)
  {
    if (pChamp1->buffer[no] == pChamp2->buffer[no]) continue;
    if (pChamp1->buffer[no] >  pChamp2->buffer[no]) return 1;

    ASSERTc (pChamp1->buffer[no] < pChamp2->buffer[no], -1, cExAlgorithm)

    return -1;
  }

  return 0;
}

// Additionne les valeurs entières non signées représentées par les champs -----
//> Champ résultat de l'addition
sChampBits sChampBits_Add
  (const sChampBits* pChamp1, //: Champ à additionner
   const sChampBits* pChamp2) //: Champ à additionner
{
  ASSERTpc (pChamp1, sChampBits_New(), cExNullPtr)
  ASSERTpc (pChamp2, sChampBits_New(), cExNullPtr)

  sChampBits _resultat;

  uint32_t _retenue = 0;

  unsigned no;

  for (no = 0; no < CHAMP_NOMBRE_UNITE; no++)
  {
    uint32_t v1 = pChamp1->buffer[no];
    uint32_t v2 = pChamp2->buffer[no];
    uint32_t v3 = v1 + v2;

    _resultat.buffer[no] = v3 + _retenue;

    if (_retenue)
    {
      _retenue =
      (v1 > CHAMP_VALEUR_MAX - v2)       ? 1 /*ret car v1+v2     > max */ :
     ((v3 > CHAMP_VALEUR_MAX - _retenue) ? 1 /*ret car v1+v2+ret > max */ : 0);
    }
    else
    {
      _retenue = (v1 > CHAMP_VALEUR_MAX - v2) ? 1 : 0;
    }
  }

  return _resultat;
}

// Test si un algorithme de traitement de champs de bits fonctionne bien     ---
// (vérifie si la retenue, grosse source d'erreurs, est bien redistribuée    ---
// entre les buffers mais ne vérifie pas si le résultat retourné est correct ---
// ...)                                                                      ---
//> Faut-il faire confiance à l'algorithme (aucun problème de retenue) ?
bool sChampBits_SelfTest
  (sChampBits (*pFonction) //: Pointeur sur la fonction à tester
  (const sChampBits*, const sChampBits*))
{
  unsigned no, nb;

  sChampBits a1, a2;
  sChampBits b1, b2;
  sChampBits c1, c2;

  for (nb = 0; nb < 1024*1024; nb++)
  {
    a1 = sChampBits_New();
    a2 = sChampBits_New();
    b1 = sChampBits_New();
    b2 = sChampBits_New();

    unsigned _nbBits1 = RAND(64);

    for (no = 0; no < _nbBits1; no++)
    {
      uint8_t noA = RAND(255-32);
      uint8_t noB = RAND(255-32);
      sChampBits_SetBit (&a1, noA+16, true);
      sChampBits_SetBit (&b1, noB+16, true);
      sChampBits_SetBit (&a2, noA+32, true);
      sChampBits_SetBit (&b2, noB+32, true);
    }

    c1 = (*pFonction) (&a1, &b1);
    c2 = (*pFonction) (&a2, &b2);

    // (a + b) << 16 = (a << 16 + b << 16) (test la retenue de l'addition)

    for (no = 0; no < CHAMP_NOMBRE_UNITE; no++)
    {
      if ((c1.buffer[no] << 16) != (c2.buffer[no] & 0xFFFF0000)) goto _pok;

      if (no == CHAMP_NOMBRE_UNITE-1) continue;

      if ((c1.buffer[no] >> 16) != (c2.buffer[no+1] & 0x0000FFFF)) goto _pok;
    }
  }

  return true;

  _pok:

  // Affiche le cas prouvant que l'addition ne fonctionne pas et retourne false

  PRINT1 ("a1 ") sChampBits_Print (&a1); PRINT1 ("\n")
  PRINT1 ("a2 ") sChampBits_Print (&a2); PRINT1 ("\n\n")
  PRINT1 ("b1 ") sChampBits_Print (&b1); PRINT1 ("\n")
  PRINT1 ("b2 ") sChampBits_Print (&b2); PRINT1 ("\n\n")
  PRINT1 ("c1 ") sChampBits_Print (&c1); PRINT1 ("\n")
  PRINT1 ("c2 ") sChampBits_Print (&c2); PRINT1 ("\n\n")

  return false;
}
