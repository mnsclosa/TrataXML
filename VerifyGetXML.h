//#pragma once

#include <cstddef>
#define OK							0
#define CHARNOTSPACEPOINTFEATURE	1
#define CHARNOTALLOWED				2
#define ENDTAGNOTEXIST				3
#define TAGNOTEXIST					4
#define TAGMANDATORY				5

/* trocar a definição de tamanho pela ultimo define criado */
#undef SIZEERRORTABLE
#define SIZEERRORTABLE TAGMANDATORY + 1


#define TAGERROR					50
#define NAMETAG						1024
#define NAMEFUNCTIONERROR			50

#define ESCFRAME					0x01
#define FIXFRAME					0x02
#define MASKFRAME					0x03

#ifdef XML
bool	flagError = false; /* indica que houve erro */

/* struct com as Tags e posições do registro XML*/
struct Recordxml
{
	char* nameSpace;	/* name space da TAG*/
	char* nameTAG;		/* nome da TAG*/
	char* TAGComplete;	/* valor completo da TAG*/
	int	  initPos;		/* localização inicial do conteúdo da TAG*/
	int	  endPos;		/* localização final do conteúdo da TAG*/
}*recordXML;

/* struct que aramzena o erro ocorrido */
struct ErrorLog
{
	size_t	numberError; /* numero do erro referenciado em errorTable */
	char*	tag;		 /* nome da TAG com erro */
	char	function[64];/* nome da função que gerou o erro */
}*errorLog;

/* mensagens de erro da montagem do registro */
const char* errorTable[SIZEERRORTABLE] = {
						{"Leitura dos registros XML executado com sucesso."},
						{"Uma Tag nao pode iniciar com numero, espaco, ponto ou traco."},
						{"Um caracter invalido esta presente no nome da TAG."},
						{"Par de Tag´s nao encontrada."},
						{"Tag solicitada nao existe."},
						{"Tag solicitada e obrigatoria."}
};
/* caracteres não permitidos na TAG */
const char  charNotAllowed[] = { '!','"','#',
								 '$','%','&',
								'\'','(',')',
								 '*','+',',',
								 '/',';','<',
								 '=','>','?',
								 '@','[','\\',
								 ']','^','/',
								 '`','{','|',
								 '}','~' };
char	tagError[TAGERROR] = { NULL }; /* nome da TAG que contem o erro */
char	nameFunctionError[NAMEFUNCTIONERROR] = { NULL }; /* nome da função que gerou o erro */
char	nameSpace[NAMETAG] = { NULL };
char	nameTag[NAMETAG] = { NULL };
char	nameEndTag[NAMETAG] = { NULL };
char	typeFRAME = 0x00; /* indica qual o tipo do messageData */
char	errorRet[NAMETAG] = { NULL };
char*	varNew;
int		posvarNew = 0;
int		recordCurrent = 0;
int		errorXML = OK; /* numero do erro */
size_t	amountTags = 0; /* quantidade de TAG´s */
size_t	amountErrors = 0; /* quantidade de errors */
int		lastReadTag = 0;/* ultima TAG lida */
#else
// Funções
extern char* GetTag( char* record,char* nameTAG,bool sequencialRead,size_t* sizeTag,bool mandatory,char* subTAG = { NULL } );
extern void  SetError( size_t error,const char* tag,const char* function );
extern char* GetError( void );
extern void ReleaseMemory( void );
extern int	GetXml( const char* record );
extern int	GetXmlESC( const char* record,int *pos );
extern int	GetXmlALL( const char* record,bool swap = false );

// variáveis
extern	bool		flagError;

extern struct		Recordxml* recordXML;
extern struct		ErrorLog* errorLog;
extern const char*	errorTable[SIZEERRORTABLE];
extern char			charNotAllowed[];
extern char			tagError[TAGERROR];
extern char			nameFunctionError[NAMEFUNCTIONERROR];
extern char			nameTag[NAMETAG];
extern char			nameEndTag[NAMETAG];
extern char			typeFRAME;
extern char			errorRet[NAMETAG];
extern char*		varNew;

extern int			posvarNew;
extern int			recordCurrent;
extern int			errorXML;
extern size_t		amountTags;
extern size_t		amountErrors;
extern int			lastReadTag;
#endif
