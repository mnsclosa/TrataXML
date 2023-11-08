
#define FIX

#include "GetFIX.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

void ReleaseMemoryFIX( void )
{
	/* libero somente se aloquei algo */
	if( ( amountFix ) > 0 )
	{
		/* somente se houver algo criado */
		if( recordFIX != NULL )
		{
			/* leio uma a uma para fazer a liberação das Tag´s.Faço um a um por garantia e por norma libero ao contrário do que foi criado */
			for( register int count = amountFix;count > -1;count-- )
			{
				free( recordFIX[count].nameFIX );
				free( recordFIX[count].valueFIX );
			}

			/* libero a struct */
			free( recordFIX );
		}
	}

	/* reinicio a posição da busca, pois estou lendo um novo valor */
	lastReadNameFix = 0;

	/* limpo a quantidade de Tags */
	amountFix = 0;
}

/*
  *
  *		GetNameFIX( char* valFIX,bool sequencialRead,bool mandatory )	Le o conteúdo da struct FIX especificada.
  *
  *		valFIX = Nome do campo FIX a ser lido.
  *		sequencialRead = garanto que a leitura será sequencial ou  não.
*		mandatory = indica que o campo FIX tem que existir.
   *
  *		retorno = retorna o valor obtido na leitura ou NULL caso contrário.
  *
  */

char* GetNameFIX( char* valFIX,bool sequencialRead,bool mandatory )
{
	char* value = NULL;
	int				val = 0;
	register int	count = 0;

	/* garanto que estou fazendo a leitura na sequencia para ganhar tempo */
	if( sequencialRead == false )
		lastReadNameFix = 0;

	/* procuro a campo FIX selecionado */
	for( count = lastReadNameFix;count < amountFix;count++ )
	{
		/* guardo o tamanho do nome a ser procurado */
		val = strlen( valFIX );

		/* procuro a campo FIX na string e verifico se o tamanho é identico para não ler strings parciais */
		if( !memcmp( recordFIX[count].nameFIX,valFIX,val ) && ( strlen( recordFIX[count].nameFIX ) == val ) )
		{
			value = (char*)calloc( static_cast<size_t>( val + 1 ),sizeof( char ) );

			memcpy( &value,&recordFIX[count].valueFIX,val );

			/* salvo a posição para dar continuidade na leitura */
			if( sequencialRead == true )
				lastReadNameFix = count;

			break;
		}
	}

	/* se não encontrei o campo FIX */
	if( count == amountFix )
	{
		flagErrorFIX = true; /* encontrei um erro */

		/* verifico se o campo é mandatorio para acertar o erro */
		if( mandatory == false )
			errorFIX = NAMENOTEXIST;
		else
			errorFIX = NAMEMANDATORY;

		errorFIX = NAMENOTEXIST;

		/* salvo o nome da função */
		memcpy( nameFIXFunctionError,"GetNameFIX",strlen( "GetNameFIX" ) );

		strcat_s( nameFIXError,valFIX );
		strcat_s( nameFIXError," - " );
	}

	return value;
}

/*
 *
 *		GetErrorFIX()	Monta uma ponteiro com o erro ocorrido.
 *
 *		passgem = Não tem.
 *
 *		retorno = retorna a string com o erro formatado
 *
 */

char* GetErrorFIX( void )
{
	char _error[500] = { NULL };

	/* monto a mensagem */
	strcat_s( _error,errorTableFIX[errorFIX] );

	/* somente se houve erro */
	if( errorFIX != OK )
	{
		strcat_s( _error," - TagFIX: " );
		strcat_s( _error,nameFIXError );
		strcat_s( _error," - Funcao: " );
		strcat_s( _error,nameFIXFunctionError );
	}

	return _error;
}

/*
  *
  *
  *		GetFIX( char *record )	Le um registro FIX. Verificando a versão para associar os devidos nomes dos campos
  *
  *		record = informação com o registro FIX.
  *		pos = se for ler GetXmlALL vem com a ultima posição lida caso contrário é zero.
  *		charswap = caracter que sera trocado, Default NULL
  *
  *		retorno = o código do erro.
  *
  */


int GetFix( const char* record,int* pos,char charswap = 0x00 )
{
	int	_size = strlen( record == NULL ? record = "0" : record ) + 1; /* pego o tamanho da string que foi passada */

	/* se existir algo para tratar */
	if( _size > 2 )
	{
		/* se for somente a leitura da TAG a partir do primeiro byte */
		if( pos == 0 )
		{
			/* limpo a quantidade de Tags */
			amountFix = 0;
		}

		/* inicializo a struct com null */
		recordFIX = (struct Recordfix*)calloc( amountFix + 1,sizeof( struct Recordfix ) );

		/* leio byte a byte do arquivo */
		for( register int count = *pos,count2 = 0;count < _size;count++ )
		{
			/* procuro o caracter de indicação de termino do campo no caso o caracter SOH */
			if( record[count] == '=' )
			{
				if( amountFix > 0 )
					recordFIX = (struct Recordfix*)realloc( recordFIX,sizeof( struct Recordfix ) * ( amountFix + 1 ) ); /* crio mais uma struct */

				/* crio o ponteiro */
				recordFIX[amountFix].nameFIX = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

				/* copio os valores do nome da TAG */
				memcpy( recordFIX[amountFix].nameFIX,nameFix,count2 );

				/* limpo o campo */
				memset( nameFix,0x00,NAMEFIX );

				count++;	/* incremento em um para não salvar o sinalo de igual */
				count2 = 0; /* inicio a posição do campo lido */

				/* procuro pelo finalizador de campo caracter SOH */
				while( count < _size )
				{
					/* se o caracter a ser trocado exitir */
					if( record[count] == charswap )
						memcpy( (char*)&record[count],"\x01",1 );

					if( record[count] == 0x01 )
					{
						/* crio o ponteiro */
						recordFIX[amountFix].valueFIX = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG */
						memcpy( recordFIX[amountFix].valueFIX,valueFix,count2 );

						/* limpo o campo */
						memset( valueFix,0x00,VALUEFIX );

						count++;	/* incremento em um para não salvar o finalizador */
						break;
					}

					/* salvo o valor lido */
					valueFix[count2++] = record[count++];
				}
				count2 = 0; /* inicio a posição do campo lido */

				amountFix++;

				/* verifico o finalizador se existir */
				if( !memcmp( "</",&record[count],2 ) )
				{
					*pos = count;
					break;
				}
			}
			/* salvo o valor lido */
			nameFix[count2++] = record[count];
		}
	}
	else
		errorFIX = FIXEMPTY; /* não existe registro para tratar */

	//for( int conta = 0;conta < amountFix - 1;conta++ )
	//{
		//char teste[15000] = { NULL };
		//int val = recordFIX[conta].endPos - recordXML[conta].initPos + 1;
		//memset( teste,0x00,15000 );
		//memcpy( &teste,&record[recordXML[conta].initPos],val );
		//printf( "%s\n",recordXML[conta].nameTAG );
		//printf( "%s\n",teste );
	//}

	return errorFIX;
}
