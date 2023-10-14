
#define FIX

#include "GetFIX.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/*
  *
  *		GetNameFIX( char* valFIX,bool sequencialRead )	Le o conteúdo da struct FIX especificada.
  *
  *		valFIX = Nome do campo FIX a ser lido.
  *		sequencialRead = garanto que a leitura será sequencial ou  não.
  *
  *		retorno = retorna o valor obtido na leitura.
  *
  */

char* GetNameFIX( char* valFIX,bool sequencialRead )
{
	char*			value = NULL;
	int				val = 0;
	register int	count = 0;

	/* garanto que estou fazendo a leitura na sequencia para ganhar tempo */
	if( sequencialRead == false )
		lastReadNameFix = 0;

	/* procuro a campo FIX selecionado */
	for( count = lastReadNameFix;count < amountFix - 1;count++ )
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
	if( count == amountFix - 1 )
	{
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
  *
  *		GetFIX( char *record )	Le um registro FIX. Verificando a versão para associar os devidos nomes dos campos
  *
  *		record = informação com o registro FIX.
  *		pos = se for ler GetXmlALL vem com a ultima posição lida caso contrário é zero.
  *
  *		retorno = o código do erro.
  *
  */


int GetFix( const char* record,int* pos )
{
	int	_size = strlen( record ) + 1; /* pego o tamanho da string que foi passada */

	/* se for somente a leitura da TAG a partir do primeiro byte */
	if( pos == 0 )
	{
		/* limpo a quantidade de Tags */
		amountFix = 1;
	}

	/* inicializo a struct com null */
	recordFIX = (struct Recordfix*)calloc( amountFix,sizeof( struct Recordfix ) );

	/* leio byte a byte do arquivo */
	for( register int count = *pos,count2 = 0;count < _size;count++ )
	{
		/* procuro o caracter de indicação de termino do campo no caso o caracter SOH */
		if( record[count] == '=' )
		{
			/* crio o ponteiro */
			recordFIX[amountFix - 1].nameFIX = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

			/* copio os valores do nome da TAG */
			memcpy( recordFIX[amountFix - 1].nameFIX,nameFix,count2 );

			/* limpo o campo */
			memset( nameFix,0x00,NAMEFIX );

			count++;	/* incremento em um para não salvar o sinalo de igual */
			count2 = 0; /* inicio a posição do campo lido */

			/* procuro pelo finalizador de campo caracter SOH */
			while( count < _size )
			{
				if( record[count] == 0x01 )
				{
					/* crio o ponteiro */
					recordFIX[amountFix - 1].valueFIX = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

					/* copio os valores do nome da TAG */
					memcpy( recordFIX[amountFix - 1].valueFIX,valueFix,count2 );

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

			/* feito desta forma para evitar o warning e para liberar a memória com mais segurança em caso de erro */
			struct Recordfix* _recordFIX = (struct Recordfix*)realloc( recordFIX,sizeof( struct Recordfix ) * amountFix ); /* crio mais uma struct */
			if( _recordFIX != NULL )
			{
				recordFIX = _recordFIX;
				memset( &recordFIX[amountFix - 1],0x00,sizeof( struct Recordfix ) );
			}
			else
				free( _recordFIX );

		}
		else if( !memcmp( "</",&record[count],2 ) )
		{
			*pos = count;
			break;
		}

		/* salvo o valor lido */
		nameFix[count2++] = record[count];
	}

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
