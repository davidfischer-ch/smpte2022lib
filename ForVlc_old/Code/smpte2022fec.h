
/******************************************************************************\
                  REPRESENTE UN PAQUET, CROSS, WAIT, BUFFER FEC

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

//------------------------------------------------------------------------------
//              PAQUET FEC
//------------------------------------------------------------------------------
#ifndef __SPAQUETFEC__
#define __SPAQUETFEC__

// Types de données ============================================================

// Type de paquet de FEC (colonne ou ligne) ------------------------------------
typedef enum { COL = 0, ROW = 1 } eFecD;

// Type d'algorithme correcteur d'erreur du paquet de FEC ----------------------
typedef enum { XOR = 0, Hamming = 1, Reed_Solomon = 2 } eFecType;

// Type numéro de séquence RTP attribuable à un paquet de FEC ------------------
typedef uint16_t sFecNo;

// Structure représentant un numéro de séquence FEC qui peut-être null ---------
typedef struct
{
  sFecNo v;    //. Valeur du numéro de séquence
  bool   null; //. True = numéro de séquence null
} sFecNx;

// Structure représentant un paquet de FEC -------------------------------------
typedef struct
{
  // TODO enlever fecNo du header FEC car fecNo = numéro de séquence RTP du
  // paquet de FEC lors de l'encapsulation RTP du payload de FEC ci-dessous

  sFecNo fecNo; //. Numéro de séquence du paquet de FEC (à enlever du headerFEC)

  /* FEC header format (RFC 2733)
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |       SNBase low bits         |        Length recovery        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |E| PT recovery |                    Mask                       |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                          TS recovery                          |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |X|D|type |index|    Offset     |      NA       |SNBase ext bits|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+ */

  struct DWORD0 { uint32_t Length_recovery : 16;
                  uint32_t SNBase_low_bits : 16; } DWORD0;

  struct DWORD1 { uint32_t Mask            : 24;
                  uint32_t PT_recovery     : 7;
                  uint32_t E               : 1;  } DWORD1;

  struct DWORD2 { uint32_t TS_recovery     : 32; } DWORD2;

  struct DWORD3 { uint32_t SNBase_ext_bits : 8;
                  uint32_t NA              : 8;
                  uint32_t Offset          : 8;
                  uint32_t index           : 3;
                  uint32_t type            : 3;
                  uint32_t D               : 1;
                  uint32_t X               : 1;  } DWORD3;

  uint8_t* resXor; //. Résultat de l'op. de xor entre paquets média protégés
}
  sPaquetFec;

// Déclaration des Constantes ==================================================

#define FEC_SNBASE_MASK 0x00FFFFFF
#define FEC_MASK_0      0
#define FEC_X_0         0
#define FEC_E_1         1
#define FEC_INDEX_XOR   0
extern const sFecNx   FEC_NX_NULL;   //. fecNx   à NULL

// Déclaration des Fonctions ===================================================

sPaquetFec* sPaquetFec_Forge (      sFecNo    pFecNo,
                                    uint16_t  pLength_recovery,
                                    sMediaNo  pSNBase,
                                    uint8_t   pPT_recovery,
                                    uint32_t  pTS_recovery,
                                    uint8_t   pL_cols,
                                    uint8_t   pD_rows,
                                    eFecD     pD,
                              const uint8_t*  pResXor);

void sPaquetFec_Release (sPaquetFec*);
sFecNx   sFecNo_to_sFecNx     (sFecNo);
#endif


//------------------------------------------------------------------------------
//              CROSS FEC
//------------------------------------------------------------------------------
#ifndef __SCROSSFEC__
#define __SCROSSFEC__

// Types de données ============================================================

// Structure liant médiaNo et les paquets de FEC colonne + ligne liés ----------
typedef struct
{
  sFecNx colNx; //. fecNo du paquet de FEC colonne "lié" au médiaNo spécifique
  sFecNx rowNx; //. fecNo du paquet de FEC ligne   "lié" au médiaNo spécifique
}
  sCrossFec;

// Déclaration des Fonctions ===================================================

sCrossFec* sCrossFec_New();
void       sCrossFec_Release (sCrossFec*);
#endif

//------------------------------------------------------------------------------
//              WAIT FEC
//------------------------------------------------------------------------------
#ifndef __SWAITFEC__
#define __SWAITFEC__

// Types de données ============================================================

// Structure stockant un paquet de FEC en attente d'être utilisé ---------------
typedef struct
{
  sFecNo     fecNo;           //. FecNo du paquet de FEC
  uint16_t   Length_recovery; //. Longueur du payload (pResXor)
  uint8_t    PT_recovery;     //. Permet de récupérer PloadType des paq. média
  uint32_t   TS_recovery;     //. Permet de récupérer TimeStamp des paq. média
  sMediaNo   SNBase;  //. MédiaNo du 1er paquet média protégé
  sMediaNo   Offset;  //. MédiaNo médias protégés = SNBase + j*Offset
  sMediaNo   NA;      //. J est entre [0 ; NA[
  uint8_t    number;  //. Nombre de paquet média manquants
  sChampBits missing; //. Chaque bit = flag (perdu/non) d'un paquet média
  eFecD      D;       //. Direction : colonne ou ligne (col,row)
  uint8_t*   resXor;  //. Résultat de l'op. xor entre paquets média protégés
}
  sWaitFec;

// Déclaration des Fonctions ===================================================

sWaitFec* sWaitFec_New     (bool pInitParams);
sWaitFec* sWaitFec_Forge   (const sPaquetFec*);
void      sWaitFec_Release (      sWaitFec*);

sMediaNx sWaitFec_GetManque (const sWaitFec*, uint8_t pNo);
bool     sWaitFec_SetManque (      sWaitFec*, uint8_t pNo, bool pValeur);
sMediaNx sWaitFec_ComputeJ  (const sWaitFec*, sMediaNo);
#endif


//------------------------------------------------------------------------------
//              BUFFER FEC
//------------------------------------------------------------------------------
#ifndef __SBUFFERFEC__
#define __SBUFFERFEC__

// Types de données ============================================================

// Structure liant paquet de FEC et paquet média manquant ----------------------
typedef struct sBufferFec
{
  sRbTree cross;   //. Key= paquetMedia.médiaNo, Val= Cross (FEC colNx ou rowNx)
  sRbTree wait[2]; //. Key= {fecNo}, Val= wait (contenu utile du FEC + etc)
} sBufferFec_t;

// Déclaration des fonctions ===================================================

sBufferFec_t *sBufferFec_New();
void sBufferFec_Release (sBufferFec_t*);

bool sBufferFec_AddCrossByReference (sBufferFec_t*, sMediaNo, sCrossFec*, bool);
sCrossFec* sBufferFec_FindCross   (const sBufferFec_t*, sMediaNo);
bool       sBufferFec_DeleteCross (      sBufferFec_t*, sMediaNo);

bool     sBufferFec_AddWaitByReference (sBufferFec_t*, sWaitFec*, bool pOver);
sWaitFec*  sBufferFec_FindWait   (const sBufferFec_t*, eFecD, sFecNo);
bool       sBufferFec_DeleteWait (      sBufferFec_t*, eFecD, sFecNo);

bool sBufferFec_DeleteCrossAndWait (sBufferFec_t*, eFecD, sFecNo);

bool       sBufferFec_InitForeachCross  (      sBufferFec_t*, bool pReverse);
bool       sBufferFec_NextForeachCross  (      sBufferFec_t*);
sMediaNo   sBufferFec_ForeachKeyCross   (const sBufferFec_t*);
sCrossFec* sBufferFec_ForeachValueCross (const sBufferFec_t*);
#endif
