
#include <cstddef>

#define OK							0
#define NAMENOTEXIST				1

/* trocar a defini��o de tamanho pela ultimo define criado */
#define SIZEERRORTABLE NAMENOTEXIST + 1

#define NAMEFIXERROR				50
#define NAMEFIXFUNCTIONERROR		50
#define NAMEFIX						1024
#define VALUEFIX					1024

#ifdef FIX

/* struct com as Tags e posi��es do registro XML*/
struct Recordfix
{
	char* nameFIX;	/* nome do campo FIX */
	char* valueFIX;	/* valor do campo FIX */
}*recordFIX;

/* mensagens de erro da montagem do registro */
const char* errorTableFIX[SIZEERRORTABLE] = {
						{"Leitura dos registros FIX executado com sucesso."},
						{"Campo FIX solicitado nao existe."}
};
char	nameFIXError[NAMEFIXERROR] = { NULL }; /* nome do campo FIX que contem o erro */
char	nameFIXFunctionError[NAMEFIXFUNCTIONERROR] = { NULL }; /* nome da fun��o que gerou o erro */

char	nameFix[NAMEFIX] = { NULL };
char	valueFix[VALUEFIX] = { NULL };

int		errorFIX = OK; /* numero do erro */
int		amountFix = 1; /* quantidade de registros FIX�s */
int		lastReadNameFix = 0; /* salva a posi��o do ultimo camo FIX lido */
#else
// Fun��es
extern char* GetNameFIX( char* nameFIX,bool sequencialRead );

extern int	GetFix( const char* record,int* pos );

// vari�veis
extern char	nameFIXError[NAMEFIXERROR]; /* nome do campo FIX que contem o erro */
extern char	nameFIXFunctionError[NAMEFIXFUNCTIONERROR]; /* nome da fun��o que gerou o erro */

extern struct		Recordfix* recordFIX;
extern const char*	errorTableFIX[SIZEERRORTABLE];

extern char			nameFix[NAMEFIX];
extern char			valueFix[VALUEFIX];
extern int			errorFIX;
extern int			amountFix;
#endif
