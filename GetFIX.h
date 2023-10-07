
#include <cstddef>

#define OK							0
#define NAMENOTEXIST					1

/* trocar a definição de tamanho pela ultimo define criado */
#define SIZEERRORTABLE NAMENOTEXIST + 1

#ifdef FIX

/* struct com as Tags e posições do registro XML*/
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
char	nameFIXError[50] = { NULL }; /* nome do campo FIX que contem o erro */
char	nameFIXFunctionError[50] = { NULL }; /* nome da função que gerou o erro */

char	nameFix[1024] = { NULL };
char	valueFix[1024] = { NULL };

int		errorFIX = OK; /* numero do erro */
int		amountFix = 1; /* quantidade de registros FIX´s */
int		lastReadNameFix = 0; /* salva a posição do ultimo camo FIX lido */
#else
// Funções
extern char* GetNameFIX( char* nameFIX,bool sequencialRead );

extern int	GetFix( const char* record );

// variáveis
extern char	nameFIXError[50]; /* nome do campo FIX que contem o erro */
extern char	nameFIXFunctionError[50]; /* nome da função que gerou o erro */

extern struct		Recordfix* recordFIX;
extern const char*	errorTableFIX[SIZEERRORTABLE];

extern char			nameFix[1024];
extern char			valueFix[1024];
extern int			errorFIX;
extern int			amountFix;
#endif
