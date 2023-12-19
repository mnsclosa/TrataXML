
#include <cstddef>

#define OK							0
#define NAMENOTEXIST				1
#define NAMEMANDATORY				2
#define FIXEMPTY					3

/* trocar a definição de tamanho pela ultimo define criado */
#undef SIZEERRORTABLE
#define SIZEERRORTABLE FIXEMPTY + 1

#define NAMEFIXERROR				50
#define NAMEFIXFUNCTIONERROR		50
#define NAMEFIX						1024
#define VALUEFIX					1024

#ifdef FIX

bool	flagErrorFIX = false; /* indica que houve erro */
/* struct com as Tags e posições do registro XML*/
struct Recordfix
{
	char* nameFIX;	/* nome do campo FIX */
	char* valueFIX;	/* valor do campo FIX */
}*recordFIX;

/* mensagens de erro da montagem do registro */
const char* errorTableFIX[SIZEERRORTABLE] = {
						{"Leitura dos registros FIX executado com sucesso."},
						{"Campo FIX solicitado nao existe."},
						{"Campo FIX solicitado e obrigatorio."},
						{"Nao existe informacao FIX para tratar."}
};
char	nameFIXError[NAMEFIXERROR] = { NULL }; /* nome do campo FIX que contem o erro */
char	nameFIXFunctionError[NAMEFIXFUNCTIONERROR] = { NULL }; /* nome da função que gerou o erro */

char	nameFix[NAMEFIX] = { NULL };
char	valueFix[VALUEFIX] = { NULL };
char	errorFix[NAMEFIX] = { NULL };

int		numberErrorFIX = OK; /* numero do erro */
int		amountFix = 0; /* quantidade de registros FIX´s */
int		lastReadNameFix = 0; /* salva a posição do ultimo camo FIX lido */
int		checkSum = 0; /* valor do checksum da mensagem */
#else
// Funções
extern void ReleaseMemoryFIX( void );
extern char* GetErrorFIX( void );
extern char* GetNameFIX( char* nameFIX,bool sequencialRead,bool mandatory );

extern int	GetFix( const char* record,int* pos,char charswap = 0x00 );

// variáveis
extern bool	flagErrorFIX; /* indica que houve erro */

extern char	nameFIXError[NAMEFIXERROR]; /* nome do campo FIX que contem o erro */
extern char	nameFIXFunctionError[NAMEFIXFUNCTIONERROR]; /* nome da função que gerou o erro */

extern struct		Recordfix* recordFIX;
extern const char*	errorTableFIX[SIZEERRORTABLE];

extern char			nameFix[NAMEFIX];
extern char			valueFix[VALUEFIX];
extern char			errorFix[NAMEFIX];
extern int			numberErrorFIX;
extern int			amountFix;
extern int			lastReadNameFix;
extern int			checkSum;
#endif
