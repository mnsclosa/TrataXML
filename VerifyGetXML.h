//#pragma once

#include <cstddef>
#define OK							0
#define CHARNOTSPACEPOINTFEATURE	1
#define CHARNOTALLOWED				2
#define ENDTAGNOTEXIST				3
#define TAGNOTEXIST					4

/* trocar a defini��o de tamanho pela ultimo define criado */
#define SIZEERRORTABLE TAGNOTEXIST + 1


#ifdef XML

/* struct com as Tags e posi��es do registro XML*/
struct Recordxml
{
	char* nameTAG;		/* nome da TAG*/
	char* TAGComplete;	/* valor completo da TAG*/
	int		initPos;		/* localiza��o incial do conte�do da TAG*/
	int		endPos;			/* localiza��o final do conte�do da TAG*/
}*recordXML;

/* mensagens de erro da montagem do registro */
const char* errorTable[SIZEERRORTABLE] = {
						{"Leitura dos registros XML executado com sucesso."},
						{"Uma Tag nao pode iniciar com numero, espaco, ponto ou traco."},
						{"Um caracter invalido esta presente no nome da TAG."},
						{"Par de Tag�s nao encontrada."},
						{"Tag solicitada nao existe."}
};
/* caracteres n�o permitidos na TAG                 ' */
const char  charNotAllowed[] = { '!','"','#','$','%','&','\'','(',')','*','+',',','/',
						   ';','<','=','>','?','@','[','\\',']','^','/','`','{','|','}','~' };
char	tagError[50] = { NULL }; /* nome da TAG que contem o erro */
char	nameFunctionError[50] = { NULL }; /* nome da fun��o que gerou o erro */
char	nameTag[1024] = { NULL };
char	nameEndTag[1024] = { NULL };
int		error = OK; /* numero do erro */
int		amountTags = 1; /* quantidade de TAG�s */
int		lastReadTag = 0;/* ultima TAG lida */
#else
// Fun��es
extern char* GetTag( char* record,char* nameTAG,bool sequencialRead,int* sizeTag,char* subTAG = { NULL } );
extern char* GetError( void );
extern void ReleaseMemory( void );
extern int	GetXml( const char* record );
extern int	GetXmlHTML( const char* record );

// vari�veis
extern struct		Recordxml* recordXML;
extern const char*	errorTable[SIZEERRORTABLE];
extern char			charNotAllowed[];
extern char			tagError[50];
extern char			nameFunctionError[50];
extern char			nameTag[1024];
extern char			nameEndTag[1024];

extern int			error;
extern int			amountTags;
extern int			lastReadTag;
#endif
