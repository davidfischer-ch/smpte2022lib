
/******************************************************************************\
                      REPRÉSENTE UN PAQUET, BUFFER MÉDIA

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
//              PAQUET MEDIA
//------------------------------------------------------------------------------
#ifndef __SPAQUETMEDIA__
#define __SPAQUETMEDIA__

// Types de données ============================================================

// Type numéro de séquence RTP attribuable à un paquet média -------------------
typedef uint16_t sMediaNo;

// Structure représentant un numéro de séquence média qui peut-être null -------
typedef struct
{
  sMediaNo v;    //. Valeur du numéro de séquence
  bool     null; //. True = numéro de séquence null
}
  sMediaNx;

// Structure représentant un paquet média (simplifié) --------------------------
typedef struct
{
  sMediaNo mediaNo;     //. Numéro de séquence du paquet média
  uint32_t timeStamp;   //. TimeStamp lié au flux
  uint8_t  payloadType; //. Type de payload
  unsigned payloadSize; //. Taille du payload
  uint8_t* payload;     //. Payload (je dirais même playload :-p)
  uint8_t* payloadWithHeader; //. Payload+RTP header
} sPaquetMedia;

// Déclaration des Constantes ==================================================

extern const sMediaNx MEDIA_NX_NULL; //. médiaNx à NULL

// Déclaration des Fonctions ===================================================
sPaquetMedia* sPaquetMedia_Forge (sMediaNo, uint32_t, uint8_t,
                                  size_t, const uint8_t*);
void sPaquetMedia_Release (      sPaquetMedia*);
sMediaNx sMediaNo_to_sMediaNx (sMediaNo);
#endif


//------------------------------------------------------------------------------
//              BUFFER MEDIA
//------------------------------------------------------------------------------
#ifndef __SBUFFERMEDIA__
#define __SBUFFERMEDIA__

// Types de données ============================================================

// Structure stockant les paquets média dans un arbre rouge-noire. Le buffer ---
// média de VLC est une liste chaînée, ce qui n'est pas optimal lors de      ---
// nombreuses manipulations du buffer. Une ancienne version avec liste       ---
// doublement chaînée est disponible au cas où (version précédente).         ---
typedef struct sBufferMedia
{
  sRbTree rbtree; //. Key = paquetMedia.mediaNo, Value = paquetMedia

  sRbNode* reading; //. Position de la lecture   (dernier lu)
  sRbNode* arrival; //. Position de la réception (dernier réceptionné)

  sMediaNx readingNx; //. Position de la lecture   (dernier médiaNo lu)
  sMediaNx arrivalNx; //. Position de la réception (dernier médiaNo réceptionné)
} sBufferMedia_t;

// Déclaration des fonctions ===================================================

sBufferMedia_t *sBufferMedia_New();
void sBufferMedia_Release (sBufferMedia_t*);

bool sBufferMedia_AddByReference (sBufferMedia_t*, sPaquetMedia*, bool pOver);

sPaquetMedia* sBufferMedia_Find (const sBufferMedia_t*, sMediaNo);

bool          sBufferMedia_InitForeach  (      sBufferMedia_t*, bool pReverse);
bool          sBufferMedia_NextForeach  (      sBufferMedia_t*);
sMediaNo      sBufferMedia_ForeachKey   (const sBufferMedia_t*);
sPaquetMedia* sBufferMedia_ForeachValue (const sBufferMedia_t*);

bool sBufferMedia_IsMediaNoInBuffer (const sBufferMedia_t*, sMediaNo);
bool sBufferMedia_ReadMedia         (sBufferMedia_t*, sMediaNo* pReadedNo);
#endif
