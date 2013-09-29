/**************************************************************************************************\
        OPTIMIZED AND CROSS PLATFORM SMPTE 2022-1 FEC LIBRARY IN C, JAVA, PYTHON, +TESTBENCH

    Description    : Generator of the test bench
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

// Variables Globales ==========================================================

static bool     optionAutoKey  = false; //. Automatiquement valider les msgs ?
static char*    optionSource   = NULL; //. Fichier source
static char*    optionDest     = NULL; //. Fichier destination
static unsigned optionLrecov   = PLDS; //. Longueur du payload (resXor)
static bool     option2DMatrix = true; //. Mode 1D ou 2D pour le fec ?
static sMediaNo optionMedia0   = 0;    //. Premier médiaNo pour un paquet média
static sFecNo   optionCol0     = 0;    //. Premier fecNo pour paquet FEC colonne
static sFecNo   optionRow0     = 0;    //. Premier fecNo pour paquet FEC ligne
static uint8_t  optionL        = 5;    //. Taille (colonne) de la matrice de FEC
static uint8_t  optionD        = 5;    //. Taille (ligne)   de la matrice de FEC
static unsigned optionLD       = 5*5;  //. Produit des deux paramètres (L*D)
static uint8_t  optionGap      = 0;    //. Décalage constant sur colonne ?
static uint8_t  payload[1460]; //. Utilisé lors du traitement (1460 = max RTP)

// Fonctions publiques =========================================================

// Affiche (et enregistre dans un fichier) le contenu des deux algorithmes -----
void AssertPrintError()
{
}

// Point d'entrée du programme -------------------------------------------------
//> Code d'erreur renvoyé au système (0 = ok)
int main (int argc, char ** argv)
{
  bool     ok;
  unsigned no;

  PRINT_INIT_COLOR()

  // Affiche le titre du logiciel
  PRINT0_CON (cConDefault, "%s", cFecGeneratorMsgTitle)

  signed sno;
  for (sno = 1; sno < argc; sno++)
  {
    char* arg = argv[sno];

    if (strcmp (arg, cLabelVerbose1) == 0)
    {
      verbose = 1;
    }
    else if (strcmp (arg, cLabelVerbose2) == 0)
    {
      verbose = 2;
    }
    else if (strcmp (arg, cLabelAutoKey) == 0)
    {
      optionAutoKey = true;
    }
    else if (strcmp (arg, cLabelAbout) == 0)
    {
      PRINT0_CON    (cConTitle,   "%s", cMsgAboutTGoal)
      PRINT0_CON    (cConDefault, "%s", cMsgAboutLGoal)
      PRINT0_CON    (cConTitle,   "%s", cMsgAboutTFunction)
      PRINT0_CON    (cConDefault, "%s", cFecGeneratorMsgAboutLFunction)
      PRINT0_CON    (cConTitle,   "%s", cTheGuyTitle)
      PRINT0_CON    (cConDefault, "%s", cTheGuyLabel)
      KeyToContinue (optionAutoKey);

      return 0;
    }
    else if (strcmp (arg, cLabelHelp) == 0)
    {
      PRINT0_CON    (cConDefault, "%s", cFecGeneratorMsgHelp)
      KeyToContinue (optionAutoKey);
      PRINT0_CON    (cConDefault, "%s", cFecGeneratorMsgHelpGap0)
      KeyToContinue (optionAutoKey);
      PRINT0_CON    (cConDefault, "%s", cFecGeneratorMsgHelpGap1)
      KeyToContinue (optionAutoKey);

      return 0;
    }
    else
    {
      char* value;

      if ((value = GetParameterValue (arg, cLabelSource, '=')) != 0)
      {
        optionSource = value;
      }
      else if ((value = GetParameterValue (arg, cLabelDest, '=')) != 0)
      {
        optionDest = value;
      }
      else if ((value = GetParameterValue (arg, cLabelLrecov, '=')) != 0)
      {
        optionLrecov = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabel2DMatrix, '=')) != 0)
      {
        unsigned dim = atoi (value);

        ASSERTc (dim >= 1 && dim <= 2, -1, cExMatrixDim)

        option2DMatrix = dim == 2 ? true : false;
      }
      else if ((value = GetParameterValue (arg, cLabelMedia0, '=')) != 0)
      {
        optionMedia0 = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabelCol0, '=')) != 0)
      {
        optionCol0 = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabelRow0, '=')) != 0)
      {
        optionRow0 = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabelL, '=')) != 0)
      {
        optionL = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabelD, '=')) != 0)
      {
        optionD = atoi (value);
      }
      else if ((value = GetParameterValue (arg, cLabelGap, '=')) != 0)
      {
        optionGap = atoi (value);
      }
      else // Un paramètre incorrect
      {
        goto __params_error;
      }
    }
  }

  if (optionSource == 0)
  {
  __params_error:
    PRINT0_CON    (cConError, "%s", cFecGeneratorMsgSyntax)
    PRINT0_CON    (cConError, "%s", cFecGeneratorMsgHelp)
    KeyToContinue (optionAutoKey);

    return 0;
  }

  optionLD = optionL * optionD;

  // Vérification de certains paramètres
  ASSERTc (optionLD == optionL * optionD, -1, cExMatrixLD)
  ASSERTc (optionL >= 1,                  -1, cExMatrixMin)
  ASSERTc (optionD >= 1,                  -1, cExMatrixMin)
  ASSERTc (optionL <= 256,                -1, cExMatrixMax)
  ASSERTc (optionD <= 256,                -1, cExMatrixMax)
  ASSERTc (optionGap < optionD,           -1, cExMatrixGap)

  // SMPTE 2022 définit quelques limites standards
  if (optionLD > 100 || optionL > 20 || optionD < 4 || optionD > 20)
  {
    PRINT0_CON    (cConWarning, "%s", cMsgSmpteStandard)
    KeyToContinue (optionAutoKey);
  }

  PRINT0_FILE
    (cFecGeneratorLogFile, "w",
     "Lrecov:%u %s media0:%u col0:%u row0:%u L:%u D:%u gap:%u\n\n",
     optionLrecov, option2DMatrix ? "2D" : "1D",
     optionMedia0, optionCol0, optionRow0, optionL, optionD, optionGap)

  // ===========================================================================

  if (optionDest == 0)
  {
    optionDest = UtilePourMoi (optionSource, cFecGeneratorDestSuffixe);
  }

  // Ouverture de la source
  FILE*    source = fopen (optionSource, "rb");
  ASSERTc (source, -1, cExSourceFile)

  // Création de la destination
  FILE*    dest = fopen (optionDest, "wb");
  ASSERTc (dest,   -1, cExDestFile)

  // Lecture de la taille du fichier source
  fseek (source, 0, SEEK_END);
  long sourceSize = ftell (source);
  long sourcePos  = 0;
  rewind (source);

  // Barre de pourcentage
  double oldPcent = 0, newPcent = 0;

  // Variables utilisées pour la génération des paquets média
  sPaquetMedia* media;
  sMediaNo      media0  = optionMedia0;
  unsigned      mediaNb = 0;

  // Variables utilisées pour la génération des paquets de FEC colonne (col)
  sPaquetFec** col    = calloc (optionL, sizeof (sPaquetFec*));
  unsigned*    colNA  = calloc (optionL, sizeof (unsigned));
  unsigned*    colGap = calloc (optionL, sizeof (unsigned));
  sFecNo       col0   = optionCol0;
  unsigned     colNo  = 0;

  ASSERTc (col,    -1, cExAllocateMemory)
  ASSERTc (colNA,  -1, cExAllocateMemory)
  ASSERTc (colGap, -1, cExAllocateMemory)

  for (no = 0; no < optionL; no++)
  {
    colGap[no] = no * optionGap; // Décalage de génération des paquets (gap)
  }

  // Variables utilisées pour la génération des paquets de FEC ligne (row)
  sPaquetFec* row   = 0;
  unsigned    rowNA = 0;
  sFecNo      row0  = optionRow0;

  PRINT0_CON     (cConDefault, "%s", cFecGeneratorMsg1of1)
  PRINT_SET_FILE (cFecGeneratorLogFile, "a")

  // BOUCLE DE CONVERSION FICHIER -> FICHIER RTP + FEC =========================

  while (true)
  {
    size_t nbLu = fread (payload, 1, optionLrecov, source); // Copie la source

    // Met à jour la barre de pourcentage
    sourcePos = ftell (source);

    PCENT ((double)sourcePos / (double)sourceSize, nbLu == 0,
           cFecGeneratorLogFile)

    if (nbLu == 0) break;

    if (nbLu < optionLrecov)
    {
      void*    ok = memset (&payload[nbLu], 0, optionLrecov - nbLu);
      ASSERTc (ok, -1, cExMediaForge)
    }

    // GÉNÉRATION DES PAQUETS MÉDIA ============================================

    // Forge le paquet média à partir du payload (fraction du fichier source)
    media = sPaquetMedia_Forge (media0, clock() / TICKS_TO_MS, PAYLOAD_TYPE,
                                        optionLrecov,          payload);
    ASSERTc (media, -1, cExMediaForge)

    // Enregistre le paquet média dans le fichier destination
    ok = sPaquetMedia_ToFile (media, dest, true);
    ASSERT (ok, -1, cExMediaToFile, media->mediaNo)

    if (mediaNb %  optionL          == 0) PRINT1("\n")
    if (mediaNb % (optionL*optionD) == 0) PRINT1("\n")
    PRINT1 (cMsgMed, media0)

    mediaNb++;

    // GÉNÉRATION DES PAQUETS DE FEC COLONNE ===================================

    if (colGap[colNo] == 0)
    {
      // Forge un paquet de FEC si nécessaire
      if (colNA[colNo] == 0)
      {
        col[colNo] = sPaquetFec_Forge (col0++, optionLrecov, media0, 0, 0,
                                       optionL, optionD, COL, 0);
        ASSERTc (col[colNo], -1, cExFecForge)
      }

      // Opération de Xor
      col[colNo]->DWORD2.TS_recovery ^= media->timeStamp;
      col[colNo]->DWORD1.PT_recovery ^= media->payloadType;

      for (no = 0; no < media->payloadSize; no++)
      {
        col[colNo]->resXor[no] ^= media->payload[no];
      }

      // Paquet de FEC terminé : l'enregistre !
      if (++colNA[colNo] == col[colNo]->DWORD3.NA)
      {
        colNA[colNo] = 0;

        ok = sPaquetFec_ToFile (col[colNo], dest);
        ASSERT (ok, -1, cExFecToFile, col[colNo]->fecNo)
        PRINT1 (cMsgCol, col[colNo]->fecNo)

        sPaquetFec_Release (col[colNo]);
      }
    }
    else
    {
      colGap[colNo]--;
    }

    // GÉNÉRATION DES PAQUETS DE FEC LIGNE =====================================

    if (option2DMatrix)
    {
      // Forge un paquet de FEC si nécessaire
      if (rowNA == 0)
      {
        row = sPaquetFec_Forge (row0++, optionLrecov, media0, 0, 0,
                                optionL, optionD, ROW, 0);
        ASSERTc (row, -1, cExFecForge)
      }

      // Opération de Xor
      row->DWORD2.TS_recovery ^= media->timeStamp;
      row->DWORD1.PT_recovery ^= media->payloadType;

      for (no = 0; no < media->payloadSize; no++)
      {
        row->resXor[no] ^= media->payload[no];
      }

      // Paquet de FEC terminé : l'enregistre !
      if (++rowNA == row->DWORD3.NA)
      {
        rowNA = 0;

        ok = sPaquetFec_ToFile (row, dest);
        ASSERT (ok, -1, cExFecToFile, row->fecNo)
        PRINT1 (cMsgRow, row->fecNo)

        sPaquetFec_Release (row);
      }
    }

    // MISE A JOUR DES COMPTEURS ===============================================

    colNo = (colNo + 1) % optionL;
    media0++;

    sPaquetMedia_Release (media);
  }

  // PAQUETS DE FEC ENCORE EN CRÉATION ? TRONQUER NA ET LES ENREGISTRER ========

  for (colNo = 0; colNo < optionL; colNo++)
  {
    if (colNA[colNo] > 0)
    {
      col[colNo]->DWORD3.NA = colNA[colNo]; // Corrige NA ("tronque" ...)

      ok = sPaquetFec_ToFile (col[colNo], dest);
      ASSERT (ok, -1, cExFecToFile, col[colNo]->fecNo)
      PRINT1 (cMsgCol, col[colNo]->fecNo)

      sPaquetFec_Release (col[colNo]);
    }
  }

  if (option2DMatrix)
  {
    if (rowNA > 0)
    {
      row->DWORD3.NA = rowNA; // Corrige NA ("tronque" ...)

      ok = sPaquetFec_ToFile (row, dest);
      ASSERT (ok, -1, cExFecToFile, row->fecNo)
      PRINT1 (cMsgRow, row->fecNo)

      sPaquetFec_Release (row);
    }
  }

  free (col);
  free (colNA);
  free (colGap);

  fclose (source);
  fclose (dest);

  // ===========================================================================

  PRINT0_CON    (cConDefault, "%s", cMsgEnded)
  KeyToContinue (optionAutoKey);

  return 0;
}
