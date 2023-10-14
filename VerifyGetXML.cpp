/* Faz a verifica��o de um arquivo XML analisando a existencia do abre e fecha das TAGs
   e faz a leitura do conte�do de cada TAG existente.
   Foi criado em "C" para se obter o m�ximo de performance
 */

#define XML /* criado para acertar para extern quando a fun��o �u vriavel n�o for local */

#include "VerifyGetXML.h"
#include "GetFIX.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

 /*
  *
  *		GetTAG( char* record,char* nameTAG,bool sequencialRead,int* sizeTag,char* subTAG = { NULL } )	Le o conte�do da TAG especificada.
  *
  *		record = informa��o com o registro XML.
  *		nameTAG = Nome da Tag a ser lida.
  *		subTAG = Le o conte�do quando este estiver dentro de uma SUBTAG (NULL por padr�o).
  *		sequencialRead = garanto que a leitura ser� sequencial.
  *		sizeTag = retorna o tamanho lido da TAG.
  *
  *		retorno = retorna o valor obtido na leitura.
  *
  */

char* GetTag( char* record,char* nameTAG,bool sequencialRead,int* sizeTag,char* subTAG = { NULL } )
{
	char* value = NULL;
	int		val = 0;
	register int count = 0;

	/* garanto que estou fazendo a leitura na sequencia para ganhar tempo */
	if( sequencialRead == false )
		lastReadTag = 0;

	/* procuro a TAG selecionada */
	for( count = lastReadTag;count < amountTags - 1;count++ )
	{
		/* procuro a TAG na string e verifico se o tamanho � identico para n�o ler strings parciais */
		if( !memcmp( recordXML[count].nameTAG,nameTAG,strlen( nameTAG ) ) && ( strlen( recordXML[count].nameTAG ) == strlen( nameTAG ) ) )
		{
			/* procuro pela subTAG se for diferente de NULL */
			if( subTAG != NULL )
			{
				while( count < amountTags )
				{
					/* procuro a subTAG na string e verifico se o tamanho � identico para n�o ler strings parciais */
					if( !memcmp( recordXML[count].nameTAG,subTAG,strlen( subTAG ) ) && ( strlen( recordXML[count].nameTAG ) == strlen( subTAG ) ) )
						break;
					count++;
				}
			}
			val = ( recordXML[count].endPos - recordXML[count].initPos ) + 1;
			value = (char*)calloc( static_cast<size_t>( val + 1 ),sizeof( char ) );

			memcpy( value,&record[recordXML[count].initPos],val );

			/* salvo a posi��o para dar continuidade na leitura */
			if( sequencialRead == true )
				lastReadTag = count;

			break;
		}
	}

	/* se n�o encontrei a TAG */
	if( count == amountTags - 1 )
	{
		error = TAGNOTEXIST;

		/* salvo o nome da fun��o */
		memcpy( nameFunctionError,"GetTag",strlen( "GetXml" ) );

		strcat_s( tagError,nameTAG );
		strcat_s( tagError," - " );
		strcat_s( tagError,subTAG == NULL ? "" : subTAG );
		value = (char*)calloc( static_cast<size_t>( 0 ),sizeof( char ) );
	}
	*sizeTag = strlen( value );
	return value;
}

/*
 *
 *		GetError()	Monta uma ponteiro com o erro ocorrido.
 *
 *		passgem = N�o tem.
 *
 *		retorno = retorna a string com o erro formatado
 *
 */

char* GetError( void )
{
	char _error[500] = { NULL };

	/* monto a mensagem */
	strcat_s( _error,errorTable[error] );
	strcat_s( _error," - Tag: " );
	strcat_s( _error,tagError );
	strcat_s( _error," - Funcao: " );
	strcat_s( _error,nameFunctionError );

	return _error;
}

/*
 *
 *		ReleaseMemory()	Libera a memoria alocada com calloc.
 *
 *		passgem = N�o tem.
 *
 *		retorno = n�o tem.
 *
 */

void ReleaseMemory( void )
{
	/* libero somente se aloquei algo */
	if( ( amountTags - 1 ) > 0 )
	{
		/* leio uma a uma para fazer a libera��o das Tag�s.Fa�o um a um por garantia e por norma libero ao contr�rio do que foi criado */
		for( register int count = amountTags - 1;count > -1;count-- )
		{
			free( recordXML[count].nameTAG );
			free( recordXML[count].TAGComplete );
		}

		/* libero a struct */
		free( recordXML );

		amountTags = 1;
	}
}

/*
 *
 *
 *		GetXml( char *record )	Le um registro XML e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
 *						Salva a posi��o de inicio e fim do conte�do a elas associada.
 *
 *		record = informa��o com o registro XML.
 *
 *		retorno = o c�digo do erro.
 *
 */


int GetXml( const char* record )
{
	bool	_nextTag = false;
	int		_size = strlen( record ) + 1;
	int		_posEndEnvelope = _size - 2;/* posiciono no fim e retorno para encontrar o inicio da TAG de fechamento */
	size_t  _amount = 0;

	/* reinicio a posi��o da busca, pois estou lendo um novo valor */
	lastReadTag = 0;

	/* libero a struct e seus componentes */
	ReleaseMemory();

	/* limpo a quantidade de Tags */
	amountTags = 1;

	/* inicializo a struct com null */
	recordXML = (struct Recordxml*)calloc( amountTags,sizeof( struct Recordxml ) );

	/* Leio a ultima TAG que � a de fechamento da primeira para diminuir o custo de olhar um a um at� o fim */
	while( _posEndEnvelope > 0 )
	{
		/* procuro os caracteres de inicio do fechamento da TAG </ */
		if( record[_posEndEnvelope] == '<' && record[_posEndEnvelope + 1] == charNotAllowed[12] )
		{
			_posEndEnvelope--;
			break;
		}
		_posEndEnvelope--;
	}

	/* leio byte a byte do arquivo */
	for( register int count = 0;count < _size;count++ )
	{
		/* procuro o caracter de abertura de TAG				'/' */
		if( record[count] == '<' && record[count + 1] != charNotAllowed[12] )
		{
			int count2 = 0;

			/* pulo o caracter de inicio de TAG "<" */
			count++;

			/* verifico se o primeiro caracter � n�mero, espa�o, ponto ou tra�o segundo as regras para uma TAG */
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
						/* salvo o nome da fun��o */
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

			/* caracteres n�o permitidos no nome da TAG !"#$%&'()*+,/ ;<=>?@[\]^`{|}~ relacionados abaixo, menos o > e o /. Fiz um a um para maximizar
			   a compara��o a posi��o 16 ficou de fora */
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
					/* caracter > ou espa�o */
					if( record[count] == charNotAllowed[16] || record[count] == ' ' )
					{
						/* crio o ponteiro */
						recordXML[amountTags - 1].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG */
						memcpy( recordXML[amountTags - 1].nameTAG,nameTag,count2 );

						memset( nameTag,0x00,NAMETAG );

						count2 = 0; /* inicio a posi��o da TAG lida */

						/* tiro o espa�o caso exista */
						if( record[count] == ' ' )
							count++;

						/* para evitar um loop infinito */
						while( count < _size )
						{
							/* procuro o caracter de fechamento da TAG */
							if( record[count] == charNotAllowed[16] )
							{
								/* crio o ponteiro */
								recordXML[amountTags - 1].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

								/* copio a Tag completa */
								memcpy( recordXML[amountTags - 1].TAGComplete,nameTag,count2 );

								/* salvo a posi��o inicial */
								recordXML[amountTags - 1].initPos = count + 1;

								/* limpo a variavel */
								memset( nameEndTag,0x00,NAMETAG );

								/* crio a TAG de fechamento */
								strcat_s( nameEndTag,"<" );
								strcat_s( nameEndTag,"/" );
								strcat_s( nameEndTag,recordXML[amountTags - 1].nameTAG );
								strcat_s( nameEndTag,">" );

								/* crio a variavel aqui para liberar mem�ria mais r�pido,retorno 2 para o caso do fechamento estar logo em seguida e
								   acrescento a TAG final do Envelope na primeira passada com a posi��o lida anteriormente */
								int pos = ( _posEndEnvelope == 0 ) ? count - 2 : _posEndEnvelope - 2;

								error = ENDTAGNOTEXIST;

								/* para evitar um loop infinito */
								while( pos < _size )
								{
									/* procuro a TAG de fechamento */
									if( memcmp( nameEndTag,&record[pos++],strlen( nameEndTag ) ) == 0 )
									{
										/* salvo a posi��o inicial */
										recordXML[amountTags - 1].endPos = pos - 2;
										error = OK;
										break;
									}
								}
								if( error == ENDTAGNOTEXIST )
								{
									/* salvo o nome da fun��o */
									memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );

									/* limpo a tagError*/
									memset( tagError,0x00,TAGERROR );

									/* salvo o nome da TAG */
									memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );

									return error;
								}

								amountTags++; /* incremento a quantidade de TAG's*/
								_posEndEnvelope = 0; /* indico que j� li o envelope*/

								/* feito desta forma para evitar o warning e para liberar a mem�ria com mais seguran�a em caso de erro*/
								struct Recordxml* _recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * amountTags ); /* crio mais uma struct */
								if( _recordXML != NULL )
								{
									recordXML = _recordXML;
									memset( &recordXML[amountTags - 1],0x00,sizeof( struct Recordxml ) );
								}
								else
									free( _recordXML );
								break;
							}
							nameTag[count2++] = record[count++];
						}
						/* reposiciono a posi��o da variavel tempor�ria no in�cio e a geral no pr�ximo */
						count2 = 0;
						count++;

						/* limpo a variavel */
						memset( nameTag,0x00,200 );

						/* indico para ler a pr�xima TAG */
						_nextTag = true;
						break;
					}
				}
				else
				{
					/* erro */
					error = CHARNOTALLOWED;

					/* salvo o nome da fun��o */
					memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
					return error;
				}
				/* salvo o valor lido */
				nameTag[count2++] = record[count++];
			}
			/* indico para procurar o pr�ximo */
			_nextTag = false;
		}
	}

	//for( int conta = 0;conta < amountTags - 1;conta++ )
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
  *						Salva a posi��o de inicio e fim do conte�do a elas associada.
  *
  *		record = informa��o com o registro XML.
  *		pos = se for ler GetXmlALL vem com a ultima posi��o lida caso contr�rio � zero.
  *
  *		retorno = o c�digo do erro.
  *
  */


int GetXmlHTML( const char* record,int* pos )
{
	bool	_nextTag = false;
	int		_size = strlen( record ) + 1; /* pego o tamanho da string que foi passada */
	int		_posEndEnvelope = strlen( record ) - 2;/* posiciono no fim e retorno para encontrar o inicio da TAG de fechamento */
	size_t  _amount = 0;

	/* se for somente a leitura da TAG a partir do primeiro byte */
	if( pos == 0 )
	{
		/* reinicio a posi��o da busca, pois estou lendo um novo valor */
		lastReadTag = 0;

		/* libero a memoria se existir algo alocado */
		ReleaseMemory();

		/* inicializo a struct com null */
		recordXML = (struct Recordxml*)calloc( amountTags,sizeof( struct Recordxml ) );

		/* Leio a ultima TAG que � a de fechamento da primeira para diminuir o custo de olhar um a um at� o fim */
		while( _posEndEnvelope > 0 )
		{
			/* procuro os caracteres de inicio do fechamento da TAG </ */
			if( !memcmp( &record[_posEndEnvelope],"&lt;/",5 ) )
			{
				_posEndEnvelope--;
				break;
			}
			_posEndEnvelope--;
		}
	}
	else
		_posEndEnvelope = 0;

	/* leio byte a byte do arquivo */
	for( register int count = *pos;count < _size;count++ )
	{
		/* procuro o caracter de abertura de TAG */
		if( !memcmp( &record[count],"&lt;",4 ) && record[count + 4] != charNotAllowed[12] )
		{
			int count2 = 0;

			/* pulo o caracter de inicio de TAG "&lt;" */
			count += 4;

			/* verifico se o primeiro caracter � n�mero, espa�o, ponto ou tra�o segundo as regras para uma TAG */
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
						/* salvo o nome da fun��o */
						memcpy( nameFunctionError,"GetXmlHTML",strlen( "GetXmlHTML" ) );
						break;
					}
					/* salvo o valor lido */
					tagError[count2++] = record[count++];
				}

				return error;
			}

			/* caracteres n�o permitidos no nome da TAG !"#$%&'()*+,/ ;<=>?@[\]^`{|}~ relacionados abaixo, menos o > e o /. Fiz um a um para maximizar
			   a compara��o a posi��o 16 ficou de fora */
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
					/* caracter > ou espa�o */
					if( !memcmp( &record[count],"&gt;",4 ) || record[count] == ' ' )
					{
						/* crio o ponteiro */
						recordXML[amountTags - 1].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG */
						memcpy( recordXML[amountTags - 1].nameTAG,nameTag,count2 );

						memset( nameTag,0x00,NAMETAG );

						count2 = 0; /* inicio a posi��o da TAG lida */

						/* tiro o espa�o caso exista */
						if( record[count] == ' ' )
							count++;

						/* para evitar um loop infinito */
						while( count < _size )
						{
							/* procuro o caracter de fechamento da TAG */
							if( !memcmp( &record[count],"&gt;",4 ) )
							{
								/* crio o ponteiro */
								recordXML[amountTags - 1].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

								/* copio a Tag completa */
								memcpy( recordXML[amountTags - 1].TAGComplete,nameTag,count2 );

								/* salvo a posi��o inicial */
								recordXML[amountTags - 1].initPos = count + 4;

								/* limpo a variavel */
								memset( nameEndTag,0x00,200 );

								/* crio a TAG de fechamento */
								strcat_s( nameEndTag,"&lt;/" );
								strcat_s( nameEndTag,recordXML[amountTags - 1].nameTAG );
								strcat_s( nameEndTag,"&gt;" );

								/* crio a variavel aqui para liberar mem�ria mais r�pido,retorno 2 para o caso do fechamento estar logo em seguida e
								   acrescento a TAG final do Envelope na primeira passada com a posi��o lida anteriormente */
								int pos = ( _posEndEnvelope == 0 ) ? count - 2 : _posEndEnvelope - 2;

								error = ENDTAGNOTEXIST;

								/* para evitar um loop infinito */
								while( pos < _size )
								{
									/* procuro a TAG de fechamento */
									if( memcmp( nameEndTag,&record[pos++],strlen( nameEndTag ) ) == 0 )
									{
										/* salvo a posi��o inicial */
										recordXML[amountTags - 1].endPos = pos - 2;
										error = OK;
										break;
									}
								}
								if( error == ENDTAGNOTEXIST )
								{
									/* salvo o nome da fun��o */
									memcpy( nameFunctionError,"GetXmlHTML",strlen( "GetXmlHTML" ) );

									/* salvo o nome da TAG */
									memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );

									return error;
								}

								amountTags++; /* incremento a quantidade de TAG's*/
								_posEndEnvelope = 0; /* indico que j� li o envelope*/

								/* feito desta forma para evitar o warning e para liberar a mem�ria com mais seguran�a em caso de erro*/
								struct Recordxml* _recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * amountTags ); /* crio mais uma struct */
								if( _recordXML != NULL )
								{
									recordXML = _recordXML;
									memset( &recordXML[amountTags - 1],0x00,sizeof( struct Recordxml ) );
								}
								else
									free( _recordXML );
								break;
							}

							///* procuro as aspas */
							//if( !memcmp( &record[count],"&quot;",6 ) )
							//{
							//}

							nameTag[count2++] = record[count++];
						}
						/* reposiciono a posi��o da variavel tempor�ria no in�cio e a geral no pr�ximo */
						count2 = 0;
						count++;

						/* limpo a variavel */
						memset( nameTag,0x00,NAMETAG );

						/* indico para ler a pr�xima TAG */
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
						/* se encontrei "xml" procuro a proxima interroga��o */
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

					/* encontrei a interroga��o */
					if( find == true )
						break;

					/* erro */
					error = CHARNOTALLOWED;

					/* salvo o nome da fun��o */
					memcpy( nameFunctionError,"GetXmlHTML",strlen( "GetXmlHTML" ) );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
					return error;
				}
				/* salvo o valor lido */
				nameTag[count2++] = record[count++];
			}
			/* indico para procurar o pr�ximo */
			_nextTag = false;
		}
		else if( !memcmp( "</",&record[count],2 ) )
		{
			*pos = count;
			break;
		}
	}

	//for( int conta = 0;conta < amountTags - 1;conta++ )
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
 *		GetXmlALL( char *record )	Le um registro XML com conte�do HTML e FIX e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
 *						Salva a posi��o de inicio e fim do conte�do a elas associada.
 *
 *		record = informa��o com o registro XML.
 *
 *		retorno = o c�digo do erro.
 *
 */


int GetXmlALL( const char* record )
{
	bool	_nextTag = false;
	int		_size = strlen( record ) + 1;
	int		_posEndEnvelope = _size - 2;/* posiciono no fim e retorno para encontrar o inicio da TAG de fechamento */
	size_t  _amount = 0;

	/* reinicio a posi��o da busca, pois estou lendo um novo valor */
	lastReadTag = 0;

	/* libero a struct e seus componentes */
	ReleaseMemory();

	/* limpo a quantidade de Tags */
	amountTags = 1;

	/* inicializo a struct com null */
	recordXML = (struct Recordxml*)calloc( amountTags,sizeof( struct Recordxml ) );

	/* Leio a ultima TAG que � a de fechamento da primeira para diminuir o custo de olhar um a um at� o fim */
	while( _posEndEnvelope > 0 )
	{
		/* procuro os caracteres de inicio do fechamento da TAG </ */
		if( !memcmp( &record[_posEndEnvelope],"</",2 ) || !memcmp( &record[_posEndEnvelope],"&lt;/",5 ) )
		{
			_posEndEnvelope--;
			break;
		}
		_posEndEnvelope--;
	}

	/* leio byte a byte do arquivo */
	for( register int count = 0;count < _size;count++ )
	{
		/* procuro o caracter de abertura de TAG */
		if( record[count] == '<' && record[count + 1] != charNotAllowed[12] )
		{
			int count2 = 0;

			/* pulo o caracter de inicio de TAG "<" */
			count++;

			/* verifico se o primeiro caracter � n�mero, espa�o, ponto ou tra�o segundo as regras para uma TAG */
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
						/* salvo o nome da fun��o */
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

			/* caracteres n�o permitidos no nome da TAG !"#$%&'()*+,/ ;<=>?@[\]^`{|}~ relacionados abaixo, menos o > e o /. Fiz um a um para maximizar
			   a compara��o a posi��o 16 ficou de fora */
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
					/* caracter > ou espa�o */
					if( record[count] == charNotAllowed[16] || record[count] == ' ' )
					{
						/* crio o ponteiro */
						recordXML[amountTags - 1].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

						/* copio os valores do nome da TAG */
						memcpy( recordXML[amountTags - 1].nameTAG,nameTag,count2 );

						memset( nameTag,0x00,NAMETAG );

						count2 = 0; /* inicio a posi��o da TAG lida */

						/* tiro o espa�o caso exista */
						if( record[count] == ' ' )
							count++;

						/* para evitar um loop infinito */
						while( count < _size )
						{
							/* procuro o caracter de fechamento da TAG */
							if( record[count] == charNotAllowed[16] )
							{
								/* crio o ponteiro */
								recordXML[amountTags - 1].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );

								/* copio a Tag completa */
								memcpy( recordXML[amountTags - 1].TAGComplete,nameTag,count2 );

								/* salvo a posi��o inicial */
								recordXML[amountTags - 1].initPos = count + 1;

								/* limpo a variavel */
								memset( nameEndTag,0x00,NAMETAG );

								/* crio a TAG de fechamento */
								strcat_s( nameEndTag,"<" );
								strcat_s( nameEndTag,"/" );
								strcat_s( nameEndTag,recordXML[amountTags - 1].nameTAG );
								strcat_s( nameEndTag,">" );

								/* crio a variavel aqui para liberar mem�ria mais r�pido,retorno 2 para o caso do fechamento estar logo em seguida e
								   acrescento a TAG final do Envelope na primeira passada com a posi��o lida anteriormente */
								int pos = ( _posEndEnvelope == 0 ) ? count - 2 : _posEndEnvelope - 2;

								error = ENDTAGNOTEXIST;

								/* para evitar um loop infinito */
								while( pos < _size )
								{
									/* procuro a TAG de fechamento */
									if( memcmp( nameEndTag,&record[pos++],strlen( nameEndTag ) ) == 0 )
									{
										/* salvo a posi��o inicial */
										recordXML[amountTags - 1].endPos = pos - 2;
										error = OK;
										break;
									}
								}
								if( error == ENDTAGNOTEXIST )
								{
									/* salvo o nome da fun��o */
									memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );

									/* limpo a tagError*/
									memset( tagError,0x00,TAGERROR );

									/* salvo o nome da TAG */
									memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );

									return error;
								}

								amountTags++; /* incremento a quantidade de TAG's*/
								_posEndEnvelope = 0; /* indico que j� li o envelope*/

								/* feito desta forma para evitar o warning e para liberar a mem�ria com mais seguran�a em caso de erro*/
								struct Recordxml* _recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * amountTags ); /* crio mais uma struct */
								if( _recordXML != NULL )
								{
									recordXML = _recordXML;
									memset( &recordXML[amountTags - 1],0x00,sizeof( struct Recordxml ) );
								}
								else
									free( _recordXML );
								break;
							}
							nameTag[count2++] = record[count++];
						}
						/* reposiciono a posi��o da variavel tempor�ria no in�cio e a geral no pr�ximo */
						count2 = 0;
						count++;

						/* limpo a variavel */
						memset( nameTag,0x00,200 );

						/* indico para ler a pr�xima TAG */
						_nextTag = true;
						break;
					}
				}
				else
				{
					/* erro */
					error = CHARNOTALLOWED;

					/* salvo o nome da fun��o */
					memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
					return error;
				}
				/* salvo o valor lido */
				nameTag[count2++] = record[count++];
			}
			/* indico para procurar o pr�ximo */
			_nextTag = false;
		}
		else if( !memcmp( &record[count],"&lt;",4 ) && record[count + 4] != charNotAllowed[12] )
		{
			GetXmlHTML( record,&count );
			typeFRAME = HTMLFRAME; /* ajusto para o HTML*/
		}
		else if( !memcmp( &record[count],"8=",2 ) && record[count - 1] != 0x01 )
		{
			GetFix( record,&count );
			typeFRAME = FIXFRAME; /* ajusto para o FIX */
		}
	}
	lastReadTag = 0;

	//for( int conta = 0;conta < amountTags - 1;conta++ )
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
