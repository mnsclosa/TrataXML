/* Faz a verificação de um arquivo XML analisando a existencia do abre e fecha das TAGs
   e faz a leitura do conteúdo de cada TAG existente.
   Foi criado em "C" para se obter o máximo de performance
 */

#define XML /* criado para acertar para extern quando a função õu vriavel não for local */

#include "VerifyGetXML.h"
#include "GetFIX.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

 /*
  *
  *		GetTAG( char* record,char* nameTAG,bool sequencialRead,int* sizeTag,bool mandatory,char* subTAG = { NULL } )	Le o conteúdo da TAG especificada.
  *
  *		record = informação com o registro XML.
  *		nameTAG = Nome da Tag a ser lida.
  *		sequencialRead = garanto que a leitura será sequencial.
  *		sizeTag = retorna o tamanho lido da TAG.
  *		mandatory = indica que a TAG tem que existir.
  *		subTAG = Le o conteúdo quando este estiver dentro de uma SUBTAG (NULL por padrão).
  *
  *		retorno = retorna o valor obtido na leitura, senão houver erro, caso contrário retornará NULL e sizeTag zero.
  *
  */

char* GetTag( char* record,char* nameTAG,bool sequencialRead,size_t* sizeTag,bool mandatory,char* subTAG = { NULL } )
{
	char* value = NULL;
	int		val = 0;
	register int count = 0;

	/* garanto que estou fazendo a leitura na sequencia para ganhar tempo */
	if( sequencialRead == false )
		lastReadTag = 0;

	/* procuro a TAG selecionada */
	for( count = lastReadTag;count < amountTags;count++ )
	{
		/* limpo o namespace */
		memset( nameSpace,0x00,NAMETAG );

		/* copio o namespace na tag de leitura se existir */
		if( strlen( recordXML[count].nameSpace ) > 0 )
		{
			strcat_s( nameSpace,recordXML[count].nameSpace );
			strcat_s( nameSpace,":" );
		}

		strcat_s( nameSpace,nameTAG );

		/* procuro a TAG na string e verifico se o tamanho é identico para não ler strings parciais */
		if( !memcmp( recordXML[count].nameTAG,nameSpace,strlen( nameSpace ) ) && ( strlen( recordXML[count].nameTAG ) == strlen( nameSpace ) ) )
		{
			/* procuro pela subTAG se for diferente de NULL */
			if( subTAG != NULL )
			{
				/* limpo o namespace */
				memset( nameSpace,0x00,NAMETAG );

				/* copio o namespace na tag de leitura se existir */
				if( strlen( recordXML[count].nameSpace ) > 0 )
				{
					strcat_s( nameSpace,recordXML[count].nameSpace );
					strcat_s( nameSpace,":" );
				}

				strcat_s( nameSpace,subTAG );

				while( count < amountTags )
				{
					/* procuro a subTAG na string e verifico se o tamanho é identico para não ler strings parciais */
					if( !memcmp( recordXML[count].nameTAG,nameSpace,strlen( nameSpace ) ) && ( strlen( recordXML[count].nameTAG ) == strlen( nameSpace ) ) )
						break;
					count++;
				}
			}
			val = ( recordXML[count].endPos - recordXML[count].initPos ) + 1;
			value = (char*)calloc( static_cast<size_t>( val + 1 ),sizeof( char ) );

			memcpy( value,&record[recordXML[count].initPos],val );

			/* salvo a posição para dar continuidade na leitura */
			if( sequencialRead == true )
				lastReadTag = count;

			error = OK;
			break;
		}
	}

	/* se não encontrei a TAG */
	if( count == amountTags )
	{
		flagError = true; /* encontrei um erro */

		/* verifico se o campo é mandatorio para acertar o erro */
		if( mandatory == false )
			error = TAGNOTEXIST;
		else
			error = TAGMANDATORY;

		/* salvo o nome da função */
		memcpy( nameFunctionError,"GetTag",strlen( "GetXml" ) );

		strcat_s( tagError,nameTAG );
		strcat_s( tagError," - " );
		strcat_s( tagError,subTAG == NULL ? "" : subTAG );
		*sizeTag = 0;
	}
	else
		*sizeTag = strlen( value );
	return value;
}

/*
 *
 *		GetError()	Monta uma ponteiro com o erro ocorrido.
 *
 *		passgem = Não tem.
 *
 *		retorno = retorna a string com o erro formatado
 *
 */

char* GetError( void )
{

	/* monto a mensagem */
	strcat_s( errorRet,errorTable[error] );
	if( error != OK )
	{
		strcat_s( errorRet," - Tag: " );
		strcat_s( errorRet,tagError );
		strcat_s( errorRet," - Funcao: " );
		strcat_s( errorRet,nameFunctionError );
	}

	return errorRet;
}

/*
 *
 *		ReleaseMemory()	Libera a memoria alocada com calloc.
 *
 *		passgem = Não tem.
 *
 *		retorno = não tem.
 *
 */

void ReleaseMemory( void )
{
	/* libero somente se aloquei algo */
	if( ( amountTags ) > 0 )
	{
		/* somente se houver algo criado */
		if( recordXML != NULL )
		{
			/* leio uma a uma para fazer a liberação das Tag´s.Faço um a um por garantia e por norma libero ao contrário do que foi criado */
			for( register int count = amountTags;count > -1;count-- )
			{
				free( recordXML[count].nameTAG );
				free( recordXML[count].TAGComplete );
			}

			/* libero a struct */
			free( recordXML );
		}
	}

	/* reinicio a posição da busca, pois estou lendo um novo valor */
	lastReadTag = 0;

	/* limpo a quantidade de Tags */
	amountTags = 0;
}

/*
 *
 *
 *		GetXml( char *record )	Le um registro XML e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
 *						Salva a posição de inicio e fim do conteúdo a elas associada.
 *
 *		record = informação com o registro XML.
 *
 *		retorno = o código do erro.
 *
 */


int GetXml( const char* record )
{
	bool	_nextTag = false;
	size_t		_size = strlen( record ) + 1;

	/* libero a struct e seus componentes */
	ReleaseMemory();

	/* inicializo a struct com null */
	recordXML = (struct Recordxml*)calloc( amountTags + 1,sizeof( struct Recordxml ) );

	/* leio byte a byte do arquivo */
	for( register int count = 0;count < _size;count++ )
	{
		/* procuro o caracter de abertura de TAG				'/' */
		if( record[count] == '<' && record[count + 1] != charNotAllowed[12] )
		{
			int count2 = 0;

			/* pulo o caracter de inicio de TAG "<" */
			count++;

			/* verifico se o primeiro caracter é número, espaço, ponto ou traço segundo as regras para uma TAG */
			if( ( record[count] > 0x2F && record[count] < 0x3A ) || record[count] == 0x20 || record[count] == 0x2E || record[count] == 0x2D )
			{
				/* pulo mais um caracter */
				count++;

				/* erro */
				error = CHARNOTSPACEPOINTFEATURE;

				/* para evitar um loop infinito e exibir a TAG errada */
				while( count < _size )
				{
					/* procuro o caracter de fechamento da TAG */
					if( record[count] == '>' || record[count] == ' ' )
					{
						/* salvo o nome da função */
						memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );
						break;
					}

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o valor lido */
					tagError[count2++] = record[count++];
				}

				return error;
			}

			/* caracteres não permitidos no nome da TAG !"#$%&'()*+,/ ;<=>?@[\]^`{|}~ relacionados abaixo, menos o > e o /. Fiz um a um para maximizar
			   a comparação a posição 16 ficou de fora */
			while( _nextTag == false )
			{
				if( record[count] != charNotAllowed[0] && record[count] != charNotAllowed[1] && record[count] != charNotAllowed[2] &&
					record[count] != charNotAllowed[3] && record[count] != charNotAllowed[4] && record[count] != charNotAllowed[5] &&
					record[count] != charNotAllowed[6] && record[count] != charNotAllowed[7] && record[count] != charNotAllowed[8] &&
					record[count] != charNotAllowed[9] && record[count] != charNotAllowed[10] && record[count] != charNotAllowed[11] &&
					record[count] != charNotAllowed[13] && record[count] != charNotAllowed[14] && record[count] != charNotAllowed[15] &&
					record[count] != charNotAllowed[17] && record[count] != charNotAllowed[18] && record[count] != charNotAllowed[19] &&
					record[count] != charNotAllowed[20] && record[count] != charNotAllowed[21] && record[count] != charNotAllowed[22] &&
					record[count] != charNotAllowed[23] && record[count] != charNotAllowed[24] && record[count] != charNotAllowed[25] &&
					record[count] != charNotAllowed[26] && record[count] != charNotAllowed[27] && record[count] != charNotAllowed[28] )
				{
					/* caracter > ou espaço */
					if( record[count] == charNotAllowed[16] || record[count] == ' ' )
					{
						/* somente após a primeiro criado */
						if( amountTags > 0 )
							recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * ( amountTags + 1 ) ); /* crio mais uma struct */

						/* crio o ponteiro */
						recordXML[amountTags].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG */
						memcpy( recordXML[amountTags].nameTAG,nameTag,count2 );

						memset( nameTag,0x00,NAMETAG );

						count2 = 0; /* inicio a posição da TAG lida */

						/* tiro o espaço caso exista */
						if( record[count] == ' ' )
							count++;

						/* para evitar um loop infinito */
						while( count < _size )
						{
							/* procuro o caracter de fechamento da TAG */
							if( record[count] == charNotAllowed[16] )
							{
								/* crio o ponteiro */
								recordXML[amountTags].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

								/* copio a Tag completa */
								memcpy( recordXML[amountTags].TAGComplete,nameTag,count2 );

								/* salvo a posição inicial */
								recordXML[amountTags].initPos = count + 1;
								recordXML[amountTags].endPos = 0;

								amountTags++; /* incremento a quantidade de TAG's*/
								break;
							}
							nameTag[count2++] = record[count++];
						}
						/* reposiciono a posição da variavel temporária no início e a geral no próximo */
						count2 = 0;
						count++;

						/* limpo a variavel */
						memset( nameTag,0x00,NAMETAG );

						/* indico para ler a próxima TAG */
						_nextTag = true;
						break;
					}
				}
				else
				{
					/* erro */
					error = CHARNOTALLOWED;

					/* salvo o nome da função */
					memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags].nameTAG,strlen( recordXML[amountTags].nameTAG ) );
					return error;
				}
				/* salvo o valor lido */
				nameTag[count2++] = record[count++];
			}
			/* indico para procurar o próximo */
			_nextTag = false;
		}
		else if( !memcmp( &record[count],"</",2 ) ) /* encontrei uma tag de fechamento, procuro a de abertura */
		{
			int count2 = 0;
			int add = count;

			/* pulo os marcadores */
			count += 2;

			/* limpo a variavel */
			memset( nameEndTag,0x00,NAMETAG );

			/* procuro o fechamento da tag */
			while( record[count] != charNotAllowed[16] || record[count] == ' ' )
				nameEndTag[count2++] = record[count++];

			/* erro */
			error = TAGNOTEXIST;

			/* procuro ao contrário para diminuir o tempo e a repetição */
			for( int pos = amountTags - 1;pos > -1;pos-- )
			{
				/* procuro a TAG de fechamento */
				if( !memcmp( nameEndTag,recordXML[pos].nameTAG,strlen( nameEndTag ) ) && !recordXML[pos].endPos )
				{
					/* salvo a posição inicial */
					recordXML[pos].endPos = add - 1;

					error = OK;
					break;
				}
			}
		}
	}

	//for( int conta = 0;conta < amountTags;conta++ )
	//{
	//	char teste[5000] = { NULL };
	//	int val = recordXML[conta].endPos - recordXML[conta].initPos + 1;
	//	memset( teste,0x00,5000 );
	//	memcpy( &teste,&record[recordXML[conta].initPos],val );
	//	printf( "%s\n",recordXML[conta].nameTAG );
	//	printf( "%s\n",teste );
	//}

	return error;
}


/*
  *
  *
  *		GetXmlHTML( char *record )	Le um registro XML em HTML e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
  *						Salva a posição de inicio e fim do conteúdo a elas associada.
  *
  *		record = informação com o registro XML.
  *		pos = se for ler GetXmlALL vem com a ultima posição lida caso contrário é zero.
  *
  *		retorno = o código do erro.
  *
  */


int GetXmlHTML( const char* record,int* pos )
{
	bool	_nextTag = false;
	size_t		_size = strlen( record ) + 1; /* pego o tamanho da string que foi passada */

	/* se for somente a leitura da TAG a partir do primeiro byte */
	if( pos == 0 )
	{
		/* libero a memoria se existir algo alocado */
		ReleaseMemory();

		/* inicializo a struct com null */
		recordXML = (struct Recordxml*)calloc( amountTags,sizeof( struct Recordxml ) );
	}

	/* leio byte a byte do arquivo */
	for( register int count = *pos;count < _size;count++ )
	{
		/* procuro o caracter de abertura de TAG */
		if( !memcmp( &record[count],"&lt;",4 ) && record[count + 4] != charNotAllowed[12] )
		{
			int count2 = 0;

			/* pulo o caracter de inicio de TAG "&lt;" */
			count += 4;

			/* verifico se o primeiro caracter é número, espaço, ponto ou traço segundo as regras para uma TAG */
			if( ( record[count] > 0x2F && record[count] < 0x3A ) || record[count] == 0x20 || record[count] == 0x2E || record[count] == 0x2D )
			{
				/* pulo mais um caracter */
				count++;

				/* erro */
				error = CHARNOTSPACEPOINTFEATURE;

				/* para evitar um loop infinito e exibir a TAG errada */
				while( count < _size )
				{
					/* procuro o caracter de fechamento da TAG */
					if( !memcmp( &record[count],"&gt;",4 ) || record[count + 4] == ' ' )
					{
						/* salvo o nome da função */
						memcpy( nameFunctionError,"GetXmlHTML",strlen( "GetXmlHTML" ) );
						break;
					}
					/* salvo o valor lido */
					tagError[count2++] = record[count++];
				}

				return error;
			}

			/* caracteres não permitidos no nome da TAG !"#$%&'()*+,/ ;<=>?@[\]^`{|}~ relacionados abaixo, menos o > e o /. Fiz um a um para maximizar
			   a comparação a posição 16 ficou de fora */
			while( _nextTag == false )
			{
				if( record[count] != charNotAllowed[0] && record[count] != charNotAllowed[1] && record[count] != charNotAllowed[2] &&
					record[count] != charNotAllowed[3] && record[count] != charNotAllowed[4] /* && record[count] != charNotAllowed[5]*/ &&
					record[count] != charNotAllowed[6] && record[count] != charNotAllowed[7] && record[count] != charNotAllowed[8] &&
					record[count] != charNotAllowed[9] && record[count] != charNotAllowed[10] && record[count] != charNotAllowed[11] &&
					record[count] != charNotAllowed[13] && record[count] != charNotAllowed[14] && record[count] != charNotAllowed[15] &&
					record[count] != charNotAllowed[17] && record[count] != charNotAllowed[18] && record[count] != charNotAllowed[19] &&
					record[count] != charNotAllowed[20] && record[count] != charNotAllowed[21] && record[count] != charNotAllowed[22] &&
					record[count] != charNotAllowed[23] && record[count] != charNotAllowed[24] && record[count] != charNotAllowed[25] &&
					record[count] != charNotAllowed[26] && record[count] != charNotAllowed[27] && record[count] != charNotAllowed[28] )
				{
					/* caracter > ou espaço */
					if( !memcmp( &record[count],"&gt;",4 ) || record[count] == ' ' )
					{
						/* somente após a primeiro criado */
						if( amountTags > 0 )
							recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * ( amountTags + 1 ) ); /* crio mais uma struct */

						/* crio o ponteiro */
						recordXML[amountTags].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG */
						memcpy( recordXML[amountTags].nameTAG,nameTag,count2 );

						memset( nameTag,0x00,NAMETAG );

						count2 = 0; /* inicio a posição da TAG lida */

						/* tiro o espaço caso exista */
						if( record[count] == ' ' )
							count++;

						/* para evitar um loop infinito */
						while( count < _size )
						{
							/* procuro o caracter de fechamento da TAG */
							if( !memcmp( &record[count],"&gt;",4 ) )
							{
								/* crio o ponteiro */
								recordXML[amountTags].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

								/* copio a Tag completa */
								memcpy( recordXML[amountTags].TAGComplete,nameTag,count2 );

								/* salvo a posição inicial */
								recordXML[amountTags].initPos = count + 4;
								recordXML[amountTags].endPos = 0;

								amountTags++; /* incremento a quantidade de TAG's*/

								break;
							}
							nameTag[count2++] = record[count++];
						}
						/* reposiciono a posição da variavel temporária no início e a geral no próximo */
						count2 = 0;
						count++;

						/* limpo a variavel */
						memset( nameTag,0x00,NAMETAG );

						/* indico para ler a próxima TAG */
						_nextTag = true;
						break;
					}
				}
				else
				{
					bool find = false;

					/* procuro pela palavra xml */
					while( count < _size )
					{
						/* se encontrei "xml" procuro a proxima interrogação */
						if( !memcmp( "xml",&record[count],3 ) )
						{
							while( count < _size )
							{
								if( !memcmp( "?",&record[count],1 ) )
								{
									while( count < _size )
									{
										/* procuro o inicializador da proxima TAG */
										if( !memcmp( "&gt;",&record[count],4 ) )
										{
											find = true;
											count += 2;
											break;
										}
										count++;
									}
								}

								/* quebro o loop */
								if( find == true )
									break;
								count++;
							}

							/* quebro o loop */
							if( find == true )
								break;
						}
						count++;
					}

					/* encontrei a interrogação */
					if( find == true )
						break;

					/* erro */
					error = CHARNOTALLOWED;

					/* salvo o nome da função */
					memcpy( nameFunctionError,"GetXmlHTML",strlen( "GetXmlHTML" ) );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags].nameTAG,strlen( recordXML[amountTags].nameTAG ) );
					return error;
				}
				/* salvo o valor lido */
				nameTag[count2++] = record[count++];
			}
			/* indico para procurar o próximo */
			_nextTag = false;
		}
		else if( !memcmp( "</",&record[count],2 ) )
		{
			*pos = count;
			break;
		}
	}

	//for( int conta = 0;conta < amountTags;conta++ )
	//{
		//char teste[15000] = { NULL };
		//int val = recordXML[conta].endPos - recordXML[conta].initPos + 1;
		//memset( teste,0x00,15000 );
		//memcpy( &teste,&record[recordXML[conta].initPos],val );
		//printf( "%s\n",recordXML[conta].nameTAG );
		//printf( "%s\n",teste );
	//}
	return error;
}

/*
 *
 *
 *		GetXmlALL( char *record )	Le um registro XML com conteúdo HTML e FIX e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
 *						Salva a posição de inicio e fim do conteúdo a elas associada.
 *
 *		record = informação com o registro XML.
 *		swap = troca os scapes pelo caracter correspondente. False por padrão
 *
 *		retorno = o código do erro.
 *
 */


int GetXmlALL( const char* record,bool swap = false )
{
	bool	_nextTag = false;
	size_t		_size = strlen( record ) + 1;

	/* libero a struct e seus componentes */
	ReleaseMemory();

	/* inicializo a struct com null */
	recordXML = (struct Recordxml*)calloc( amountTags + 1,sizeof( struct Recordxml ) );

	/* leio byte a byte do arquivo */
	for( register int count = 0;count < _size;count++ )
	{
		/* procuro o caracter de abertura de TAG */
		if( record[count] == '<' && record[count + 1] != charNotAllowed[12] )
		{
			int count2 = 0;

			/* pulo o caracter de inicio de TAG "<" */
			count++;

			/* verifico se o primeiro caracter é número, espaço, ponto ou traço segundo as regras para uma TAG */
			if( ( record[count] > 0x2F && record[count] < 0x3A ) || record[count] == 0x20 || record[count] == 0x2E || record[count] == 0x2D )
			{
				/* pulo mais um caracter */
				count++;

				/* erro */
				error = CHARNOTSPACEPOINTFEATURE;

				/* para evitar um loop infinito e exibir a TAG errada */
				while( count < _size )
				{
					/* procuro o caracter de fechamento da TAG */
					if( record[count] == '>' || record[count] == ' ' )
					{
						/* salvo o nome da função */
						memcpy( nameFunctionError,"GetXmlALL",strlen( "GetXmlALL" ) );
						break;
					}

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o valor lido */
					tagError[count2++] = record[count++];
				}

				return error;
			}

			/* caracteres não permitidos no nome da TAG !"#$%&'()*+,/ ;<=>?@[\]^`{|}~ relacionados abaixo, menos o > e o /. Fiz um a um para maximizar
			   a comparação a posição 16 ficou de fora */
			while( _nextTag == false )
			{
				if( record[count] != charNotAllowed[0] && record[count] != charNotAllowed[1] && record[count] != charNotAllowed[2] &&
					record[count] != charNotAllowed[3] && record[count] != charNotAllowed[4] && record[count] != charNotAllowed[5] &&
					record[count] != charNotAllowed[6] && record[count] != charNotAllowed[7] && record[count] != charNotAllowed[8] &&
					record[count] != charNotAllowed[9] && record[count] != charNotAllowed[10] && record[count] != charNotAllowed[11] &&
					record[count] != charNotAllowed[13] && record[count] != charNotAllowed[14] && record[count] != charNotAllowed[15] &&
					record[count] != charNotAllowed[17] && record[count] != charNotAllowed[18] && record[count] != charNotAllowed[19] &&
					record[count] != charNotAllowed[20] && record[count] != charNotAllowed[21] && record[count] != charNotAllowed[22] &&
					record[count] != charNotAllowed[23] && record[count] != charNotAllowed[24] && record[count] != charNotAllowed[25] &&
					record[count] != charNotAllowed[26] && record[count] != charNotAllowed[27] && record[count] != charNotAllowed[28] )
				{
					/* procuro pela informação de name space */
					if( record[count] == ':' )
						memcpy( nameSpace,nameTag,strlen( nameTag ) );

					/* caracter > ou espaço */
					if( record[count] == charNotAllowed[16] || record[count] == ' ' )
					{
						if( amountTags > 0 )
							recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * ( amountTags + 1 ) ); /* crio mais uma struct */

						/* crio o ponteiro */
						recordXML[amountTags].nameSpace = (char*)calloc( static_cast<size_t>( strlen( nameSpace ) + 1 ),sizeof( char ) );
						recordXML[amountTags].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG e o namespace */
						memcpy( recordXML[amountTags].nameSpace,nameSpace,strlen( nameSpace ) );
						memcpy( recordXML[amountTags].nameTAG,nameTag,count2 );

						memset( nameSpace,0x00,NAMETAG );
						memset( nameTag,0x00,NAMETAG );

						count2 = 0; /* inicio a posição da TAG lida */

						/* tiro o espaço caso exista */
						if( record[count] == ' ' )
							count++;

						/* para evitar um loop infinito */
						while( count < _size )
						{
							/* procuro o caracter de fechamento da TAG */
							if( record[count] == charNotAllowed[16] )
							{
								/* crio o ponteiro */
								recordXML[amountTags].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

								/* copio a Tag completa */
								memcpy( recordXML[amountTags].TAGComplete,nameTag,count2 );

								/* salvo a posição inicial e zero a final */
								recordXML[amountTags].initPos = count + 1;
								recordXML[amountTags].endPos = 0;

								amountTags++; /* incremento a quantidade de TAG's*/

								break;
							}
							nameTag[count2++] = record[count++];
						}
						/* reposiciono a posição da variavel temporária no início e a geral no próximo */
						count2 = 0;
						count++;

						/* limpo a variavel */
						memset( nameTag,0x00,NAMETAG );

						/* indico para ler a próxima TAG */
						_nextTag = true;
						break;
					}
				}
				else if( record[count] == charNotAllowed[17] ) /* verifico se não é interrogação */
				{
					/* pulo e deixo para o loop acertar o finalizador */
					if( !memcmp( &record[count],"?xml",4 ) )
					{
						count += 4;

						/* indico para ler a próxima TAG */
						_nextTag = true;
						break;
					}
				}
				else
				{
					/* erro */
					error = CHARNOTALLOWED;

					/* salvo o nome da função */
					memcpy( nameFunctionError,"GetXmlALL",strlen( "GetXmlALL" ) );

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
					return error;
				}
				/* salvo o valor lido */
				nameTag[count2++] = record[count++];
			}
			/* indico para procurar o próximo */
			_nextTag = false;
		}
		else if( !memcmp( &record[count],"</",2 ) ) /* encontrei uma tag de fechamento, procuro a de abertura */
		{
			int count2 = 0;
			int add = count;

			/* pulo os marcadores */
			count += 2;

			/* limpo a variavel */
			memset( nameEndTag,0x00,NAMETAG );

			/* procuro o fechamento da tag */
			while( record[count] != charNotAllowed[16] || record[count] == ' ' )
				nameEndTag[count2++] = record[count++];

			/* erro */
			error = TAGNOTEXIST;

			/* procuro ao contrário para diminuir o tempo e a repetição */
			for( int pos = amountTags - 1;pos > -1;pos-- )
			{
				/* procuro a TAG de fechamento */
				if( !memcmp( nameEndTag,recordXML[pos].nameTAG,( strlen( nameEndTag ) ) &&
					strlen( nameEndTag ) == strlen( recordXML[pos].nameTAG ) ) && !recordXML[pos].endPos )
				{
					/* salvo a posição inicial */
					recordXML[pos].endPos = add - 1;

					error = OK;
					break;
				}
			}
			/* se houve erro não continuo */
			if( error != OK )
			{
				/* salvo o nome da função */
				memcpy( nameFunctionError,"GetXmlALL",strlen( "GetXmlALL" ) );

				/* limpo a tagError*/
				memset( tagError,0x00,TAGERROR );

				/* salvo o nome da TAG */
				memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
				return error;
			}
		}

		/* se for para trcar os scapes */
		if( swap == true )
		{
			if( !memcmp( &record[count],"&",1 ) )
			{
				/* faço a troca do caracter scape */
				if( !memcmp( &record[count + 1],"l",1 ) ) /* troco &lt; por < */
				{
					memcpy( (char*)&record[count],"<",1 );
					memcpy( (char*)&record[count + 1],(char*)&record[count + 4],_size - count );
				}
				else if( !memcmp( &record[count + 1],"g",1 ) ) /* troco &gt; por > */
				{
					memcpy( (char*)&record[count],">",1 );
					memcpy( (char*)&record[count + 1],(char*)&record[count + 4],_size - count );
				}
				else if( !memcmp( &record[count + 1],"q",1 ) ) /* troco o &quot por " */
				{
					memcpy( (char*)&record[count],(char*)&charNotAllowed[1],1 );
					memcpy( (char*)&record[count + 1],(char*)&record[count + 6],_size - count );
				}

				/* elimino os caracteres removidos do total */
				_size -= 3;

				/* retorno 1 para fazer a leitura da TAG */
				//count -= 1;
			}
		}
		else if( !memcmp( &record[count],"&lt;",4 ) && record[count + 4] != charNotAllowed[12] )
		{
			GetXmlHTML( record,&count );
			typeFRAME = HTMLFRAME; /* ajusto para o HTML*/
		}

		if( !memcmp( &record[count],"8=",2 ) && record[count - 1] != 0x01 )
		{
			ReleaseMemoryFIX();
			GetFix( record,&count );
			typeFRAME = FIXFRAME; /* ajusto para o FIX */
		}
	}
	lastReadTag = 0;

	//for( int conta = 0;conta < amountTags;conta++ )
	//{
	//	char teste[5000] = { NULL };
	//	int val = recordXML[conta].endPos - recordXML[conta].initPos + 1;
	//	memset( teste,0x00,5000 );
	//	memcpy( &teste,&record[recordXML[conta].initPos],val );
	//	printf( "%s\n",recordXML[conta].nameTAG );
	//	printf( "%s\n",teste );
	//}

	return error;
}
