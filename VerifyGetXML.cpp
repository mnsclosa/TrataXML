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
  *		SetError()	Monta uma struct com os erros ocorridos.
  *
  *		error = numero do erro que ocorreu.
  *		tag = nome da tag com problema.
  *		function = nome da funçao que ocorreu o erro.
  *
  *		retorno = Nao tem;
  *
  */

void SetError( size_t error,const char* tag,const char* function )
{
	/* inicializo a struct com null */
	if( errorLog == NULL )
	{
		errorLog = (struct ErrorLog*)calloc( amountErrors + 1,sizeof( struct ErrorLog ) );

		/* indico que houve erro */
		flagError = true;
	}
	else
		errorLog = (struct ErrorLog*)realloc( errorLog,sizeof( struct ErrorLog ) * ( amountErrors + 1 ) ); /* crio mais uma struct */

	/* somente se consegui criar a struct de erro */
	if( errorLog != NULL )
	{
		/* crio a tag com erro */
		errorLog[amountErrors].tag = (char*)calloc( static_cast<size_t>( strlen( tag ) + 1 ),sizeof( char ) );
		memcpy( errorLog[amountErrors].tag,tag,strlen( tag ) + 1 );

		/* guardo o erro ocorrido */
		errorLog[amountErrors].numberError = error;
		memset( errorLog[amountErrors].function,0x00,64 );
		memcpy( errorLog[amountErrors].function,function,strlen( function ) );
	}

	/* deixo pronto para o proximo */
	amountErrors++;
	return;
}

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

			errorXML = OK;
			break;
		}
	}

	/* se não encontrei a TAG */
	if( count == amountTags )
	{
		flagError = true; /* encontrei um erro */

		/* verifico se o campo é mandatorio para acertar o erro */
		if( mandatory == false )
			errorXML = TAGNOTEXIST;
		else
			errorXML = TAGMANDATORY;

		///* salvo o nome da função */
		//memcpy( nameFunctionError,"GetTag",strlen( "GetXml" ) );

		//strcat_s( tagError,nameTAG );
		//strcat_s( tagError," - " );
		//strcat_s( tagError,subTAG == NULL ? "" : subTAG );
		SetError( errorXML,nameTAG,"GetTag" );

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
	strcat_s( errorRet,errorTable[errorXML] );
	if( errorXML != OK )
	{
		memset( errorRet,0x00,NAMETAG );
		for( int count = 0;count < amountTags;count++ )
		{
			//itoa( errorLog[count].numberError,errorRet,10 );
			strcat_s( errorRet," - " );
			strcat_s( errorRet,errorLog[count].tag );
			strcat_s( errorRet," - " );
			strcat_s( errorRet,errorTable[errorLog[count].numberError] );
			strcat_s( errorRet," - " );
			strcat_s( errorRet,errorLog[count].function );
		}
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
	if( ( amountTags ) > 0 || recordXML != NULL )
	{
		/* somente se houver algo criado */
		if( recordXML != NULL )
			/* libero a struct */
			free( recordXML );
	}

	/* reinicio a posição da busca, pois estou lendo um novo valor */
	lastReadTag = 0;

	/* limpo a quantidade de Tags */
	amountTags = 0;

	amountErrors = 0;
	errorXML = OK;
	posvarNew = 0;
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
	bool	_tagxmlVersion = false;
	char	_localChar = {};
	size_t	_size = strlen( record ) + 1;

	/* libero a struct e seus componentes */
	ReleaseMemory();

	/* inicializo o ponteiro com null */
	varNew = (char*)calloc( _size,sizeof( char* ) );

	/* verifico se foi criada */
	if( varNew != NULL )
	{
		/* inicializo a struct com null */
		recordXML = (struct Recordxml*)calloc( amountTags + 1,sizeof( struct Recordxml ) );

		/* leio byte a byte do arquivo */
		for( register int count = 0;count < _size;count++ )
		{
			/* procuro o caracter de abertura da TAG de abertura */
			if( ( record[count] == '<' && record[count + 1] != '/' && record[count + 1] != '!' ) || ( !memcmp( &record[count],"&lt;",4 ) && record[count + 4] != '/' ) )
			{
				/* forço o valor lido */
				_localChar = '<';

				/* substituo o escape */
				if( !memcmp( &record[count],"&lt;",4 ) && ( record[count + 5] != '/' || record[count + 5] == '?' ) )
				{
					if( record[count + 4] == '?' )
					{
						_tagxmlVersion = true;
						count--;
					}
					count += 4;
				}
				else
					count++;

				/* salvo o valor lido e pulo o < */
				varNew[posvarNew++] = _localChar;

				/* verifico se o primeiro caracter é número, espaço, ponto ou traço segundo as regras para uma TAG */
				if( ( record[count] >= '0' && record[count] <= '9' ) || record[count] == ' ' || record[count] == '-' || record[count] == '.' )
				{
					/* ajusto o numero do erro */
					errorXML = CHARNOTSPACEPOINTFEATURE;
					memcpy( nameTag,&record[count],20 );
					SetError( errorXML,nameTag,"GetXml" );
					break;
				}
				else if( _tagxmlVersion == false )
				{
					/* somente após a primeiro criado */
					if( amountTags > 0 )
						recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * ( amountTags + 1 ) ); /* crio mais uma struct */

					bool tagWriteNameSpace = false;
					bool tagRead = false;
					int count1 = 0; /* auxiliar */

					do
					{
						_localChar = record[count];

						/* gravo os valores relacionados ao nome da TAG */
						switch( _localChar )
						{
							/* procuro pela informação de namespace */
							case ':':
							{
								/* indico que ainda não li a TAG */
								if( tagRead == false )
								{
									/* crio o ponteiro */
									recordXML[amountTags].nameSpace = (char*)calloc( static_cast<size_t>( strlen( nameTag ) + 1 ),sizeof( char ) );
									memcpy( recordXML[amountTags].nameSpace,nameTag,strlen( nameTag ) );
									tagWriteNameSpace = true;
								}
								nameTag[count1++] = _localChar;
							}
							break;
							case ' ':
							case '>':
							case '&':
							{
								/* substituo o escape */
								if( !memcmp( &record[count],"&gt;",4 ) )
								{
									/* salvo o valor lido */
									_localChar = '>';
									count += 3;
								}

								if( tagRead == false )
								{
									/* crio o namespace se não existir */
									if( tagWriteNameSpace == false )
										recordXML[amountTags].nameSpace = (char*)calloc( static_cast<size_t>( 1 ),sizeof( char ) );

									/* crio o ponteiro e limpo a variavel */
									recordXML[amountTags].nameTAG = (char*)calloc( static_cast<size_t>( count + 1 ),sizeof( char ) );
									memcpy( recordXML[amountTags].nameTAG,nameTag,count );
									memset( nameTag,0x00,NAMETAG );
									count1 = 0;
									tagRead = true;
								}
								if( _localChar == '>' )
								{
									/* crio o ponteiro e limpo a variavel */
									recordXML[amountTags].TAGComplete = (char*)calloc( static_cast<size_t>( count + 1 ),sizeof( char ) );
									memcpy( recordXML[amountTags].TAGComplete,nameTag,count );
									memset( nameTag,0x00,NAMETAG );

									/* salvo a posição inicial */
									recordXML[amountTags].initPos = posvarNew + 1;

									/* se existir o finalizador sem a TAG */
									if( !memcmp( &record[count - 1],"/>",2 ) )
										recordXML[amountTags].endPos = posvarNew + 1;
									else
										recordXML[amountTags].endPos = 0;

									amountTags++; /* incremento a quantidade de TAG's*/

									/* retorno em 1 para não perder o próximo caracter */
									count--;
								}
								else
									nameTag[count1++] = _localChar;
							}
							break;
							default:
							{
								if( tagRead == false )
								{
									/* caracteres não permitidos no nome da TAG, menos o > e o /. */
									if( record[count] == '!' || record[count] == '"' || record[count] == '#' || record[count] == '$' || record[count] == '%' ||
										record[count] == '&' || record[count] == '\'' || record[count] == '(' || record[count] == ')' || record[count] == '*' ||
										record[count] == '+' || record[count] == ',' || record[count] == ';' || record[count] == '=' || record[count] == '<' ||
										record[count] == '?' || record[count] == '@' || record[count] == '[' || record[count] == '\\' || record[count] == ']' ||
										record[count] == '^' || record[count] == '/' || record[count] == '`' || record[count] == '{' || record[count] == '|' ||
										record[count] == '}' || record[count] == '~' || record[count] == '>' )
									{
										/* indico que houve erro */
										flagError = true;

										/* ajusto o numero do erro */
										errorXML = CHARNOTALLOWED;
										SetError( errorXML,nameTag,"GetXml" );
									}
								}
								nameTag[count1++] = _localChar;
							}
							break;
						}

						/* salvo o valor lido */
						varNew[posvarNew++] = _localChar;

						/* count++ < _size somente para não ficar infinitamente e não encontrar o caracter de fechamento da TAG */
					} while( count++ < _size && _localChar != '>' && errorXML == OK );
				}
			}/* procuro pelos comentarios */
			else if( !memcmp( &record[count],"<!",2 ) || !memcmp( &record[count],"&lt;!",5 ) )
			{
				while( count++ < _size )
				{
					if( !memcmp( &record[count],"->",2 ) )
					{
						count += 2;
						break;
					}
				}
			}/* procuro os caracteres de abertura da TAG de fechamento */
			else if( !memcmp( &record[count],"</",2 ) || !memcmp( &record[count],"&lt;",5 ) )
			{
				int count1 = 0;
				int count2 = 0;

				/* limpo a variavel */
				memset( nameEndTag,0x00,NAMETAG );
				_tagxmlVersion = false;

				/* substituo o escape */
				if( !memcmp( &record[count],"&lt;/",5 ) )
				{
					count += 5;
					count2 += 3;

					varNew[posvarNew++] = '<';
					varNew[posvarNew++] = '/';

					/* procuro o fechamento da tag */
					while( memcmp( &record[count],"&gt;",4 ) )
					{
						/* salvo o valor lido */
						_localChar = record[count];
						varNew[posvarNew++] = _localChar;
						nameEndTag[count1++] = record[count++];
						count2++;
					}
				}
				else
				{
					/* pulo os marcadores */
					count += 2;
					count2 += 3;

					/* salvo o valor lido */
					varNew[posvarNew++] = '<';
					varNew[posvarNew++] = '/';

					/* procuro o fechamento da tag */
					while( record[count] != '>' || record[count] == ' ' )
					{
						/* salvo o valor lido */
						_localChar = record[count];
						varNew[posvarNew++] = _localChar;
						nameEndTag[count1++] = record[count++];
						count2++;
					}
				}

				/* erro */
				errorXML = TAGNOTEXIST;
				flagError = true;

				/* procuro ao contrário para diminuir o tempo e a repetição */
				for( int pos = amountTags - 1;pos > -1;pos-- )
				{
					/* procuro a TAG de fechamento */
					if( !memcmp( nameEndTag,recordXML[pos].nameTAG,strlen( nameEndTag ) ) && strlen( nameEndTag ) == strlen( recordXML[pos].nameTAG ) && !recordXML[pos].endPos )
					{
						/* salvo a posição inicial */
						recordXML[pos].endPos = posvarNew - count2;

						errorXML = OK;
						flagError = false;
						break;
					}
				}

				/* se houve erro não continuo */
				if( errorXML != OK )
				{
					/* salvo o nome da função */
					memcpy( nameFunctionError,"GetXmlALL",strlen( "GetXmlALL" ) );

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o nome da TAG */
					//memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
					SetError( errorXML,nameEndTag,"GetXml" );
					break;
				}
				/* retorno em 1 para não perder o próximo caracter */
				count--;

			}/* procuro o caracter de fechamento da TAG */
			else if( !memcmp( &record[count],">",1 ) || !memcmp( &record[count],"&gt;",4 ) )
			{
				_tagxmlVersion = false;

				/* substituo o escape */
				if( !memcmp( &record[count],"&gt;",4 ) )
					count += 3;

				/* salvo o valor lido */
				varNew[posvarNew++] = '>';

			}/* troco o caracter xD = CR */
			else if( !memcmp( &record[count],"&#xD;",5 ) )
			{
				_tagxmlVersion = false;

				/* salvo o valor lido */
				_localChar = '\r';
				count += 5;
				varNew[posvarNew++] = _localChar;
			}/* troco o caracter xA = LF */
			else if( !memcmp( &record[count],"&#xA;",5 ) )
			{
				_tagxmlVersion = false;

				/* salvo o valor lido */
				_localChar = '\n';
				count += 5;
				varNew[posvarNew++] = _localChar;
			}
			else if( !memcmp( &record[count],"8=FIX",5 ) )
			{
				typeFRAME = FIXFRAME; /* ajusto para o FIX */
				GetFix( record,varNew,&count );
				posvarNew = count;

				/* retorno em 1 caracter para não perder o < */
				count--;
			}
			else
				varNew[posvarNew++] = record[count];
		}
	}

	/* limpo e copio o novo valor */
	memset( (char*)record,0x00,_size );
	memcpy( (char*)record,varNew,posvarNew );

	/* libero varNew */
	free( varNew );

	return errorXML;
}

//int GetXml( const char* record )
//{
//	bool	_nextTag = false;
//	size_t		_size = strlen( record ) + 1;
//
//	/* libero a struct e seus componentes */
//	ReleaseMemory();
//
//	/* inicializo a struct com null */
//	recordXML = (struct Recordxml*)calloc( amountTags + 1,sizeof( struct Recordxml ) );
//
//	/* leio byte a byte do arquivo */
//	for( register int count = 0;count < _size;count++ )
//	{
//		/* procuro o caracter de abertura de TAG */
//		if( ( record[count] == '<' && record[count + 1] != '/' ) || ( !memcmp( &record[count],"&lt;",4 ) && record[count + 5] != '/' ) )
//		{
//			int count2 = 0;
//
//			/* pulo o caracter de inicio de TAG "<" */
//			count++;
//
//			/* verifico se o primeiro caracter é número, espaço, ponto ou traço segundo as regras para uma TAG */
//			if( ( record[count] >= '0' && record[count] <= '9' ) || record[count] == ' ' || record[count] == '-' || record[count] == '.' )
//			{
//				/* pulo mais um caracter */
//				count++;
//
//				/* erro */
//				errorXML = CHARNOTSPACEPOINTFEATURE;
//
//				/* para evitar um loop infinito e exibir a TAG errada */
//				while( count < _size )
//				{
//					/* procuro o caracter de fechamento da TAG */
//					if( record[count] == '>' || record[count] == ' ' )
//					{
//						/* salvo o nome da função */
//						memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );
//						break;
//					}
//
//					/* limpo a tagError*/
//					memset( tagError,0x00,TAGERROR );
//
//					/* salvo o valor lido */
//					tagError[count2++] = record[count++];
//				}
//
//				return errorXML;
//			}
//
//			/* caracteres não permitidos no nome da TAG, menos o > e o /. */
//			while( _nextTag == false )
//			{
//				if( record[count] != '!' && record[count] != '"' && record[count] != '#' && record[count] != '$' && record[count] != '%' &&
//					record[count] != '&' && record[count] != '\'' && record[count] != '(' && record[count] != ')' && record[count] != '*' &&
//					record[count] != '+' && record[count] != ',' && record[count] != ';' && record[count] != '<' && record[count] != '=' &&
//					record[count] != '?' && record[count] != '@' && record[count] != '[' && record[count] != '\\' && record[count] != ']' &&
//					record[count] != '^' && record[count] != '/' && record[count] != '`' && record[count] != '{' && record[count] != '|' &&
//					record[count] != '}' && record[count] != '~' )
//				{
//					/* procuro pela informação de name space */
//					if( record[count] == ':' )
//						memcpy( nameSpace,nameTag,strlen( nameTag ) );
//
//					/* caracter > ou espaço */
//					if( record[count] == '>' || record[count] == ' ' )
//					{
//						/* somente após a primeiro criado */
//						if( amountTags > 0 )
//							recordXML = (struct Recordxml*)realloc( recordXML,sizeof( struct Recordxml ) * ( amountTags + 1 ) ); /* crio mais uma struct */
//
//						/* crio o ponteiro */
//						recordXML[amountTags].nameSpace = (char*)calloc( static_cast<size_t>( strlen( nameSpace ) + 1 ),sizeof( char ) );
//						recordXML[amountTags].nameTAG = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );
//
//						/* copio os valores do nome da TAG e o namespace */
//						memcpy( recordXML[amountTags].nameSpace,nameSpace,strlen( nameSpace ) );
//						memcpy( recordXML[amountTags].nameTAG,nameTag,count2 );
//
//						memset( nameSpace,0x00,NAMETAG );
//						memset( nameTag,0x00,NAMETAG );
//
//						count2 = 0; /* inicio a posição da TAG lida */
//
//						/* tiro o espaço caso exista */
//						if( record[count] == ' ' )
//							count++;
//
//						/* para evitar um loop infinito */
//						while( count < _size )
//						{
//							/* procuro o caracter de fechamento da TAG */
//							if( record[count] == '>' )
//							{
//								/* crio o ponteiro */
//								recordXML[amountTags].TAGComplete = (char*)calloc( static_cast<size_t>( count2 + 1 ),sizeof( char ) );
//
//								/* copio a Tag completa */
//								memcpy( recordXML[amountTags].TAGComplete,nameTag,count2 );
//
//								/* salvo a posição inicial */
//								recordXML[amountTags].initPos = count + 1;
//								recordXML[amountTags].endPos = 0;
//
//								amountTags++; /* incremento a quantidade de TAG's*/
//								break;
//							}
//							nameTag[count2++] = record[count++];
//						}
//						/* reposiciono a posição da variavel temporária no início e a geral no próximo */
//						count2 = 0;
//						count++;
//
//						/* limpo a variavel */
//						memset( nameTag,0x00,NAMETAG );
//
//						/* indico para ler a próxima TAG */
//						_nextTag = true;
//						break;
//					}
//				}
//				else
//				{
//					/* erro */
//					errorXML = CHARNOTALLOWED;
//
//					/* salvo o nome da função */
//					memcpy( nameFunctionError,"GetXml",strlen( "GetXml" ) );
//
//					/* limpo a tagError*/
//					memset( tagError,0x00,TAGERROR );
//
//					/* salvo o nome da TAG */
//					memcpy( tagError,recordXML[amountTags].nameTAG,strlen( recordXML[amountTags].nameTAG ) );
//					return errorXML;
//				}
//
//				/* salvo o valor lido */
//				nameTag[count2++] = record[count++];
//			}
//			/* indico para procurar o próximo */
//			_nextTag = false;
//		}
//		else if( !memcmp( &record[count],"</",2 ) ) /* encontrei uma tag de fechamento, procuro a de abertura */
//		{
//			int count2 = 0;
//			int add = count;
//
//			/* pulo os marcadores */
//			count += 2;
//
//			/* limpo a variavel */
//			memset( nameEndTag,0x00,NAMETAG );
//
//			/* procuro o fechamento da tag */
//			while( record[count] != '>' || record[count] == ' ' )
//			{
//				varNew[posvarNew++] = record[count];
//				nameEndTag[count2++] = record[count++];
//			}
//
//			/* erro */
//			errorXML = TAGNOTEXIST;
//
//			/* procuro ao contrário para diminuir o tempo e a repetição */
//			for( int pos = amountTags - 1;pos > -1;pos-- )
//			{
//				/* procuro a TAG de fechamento */
//				if( !memcmp( nameEndTag,recordXML[pos].nameTAG,strlen( nameEndTag ) ) && !recordXML[pos].endPos )
//				{
//					/* salvo a posição inicial */
//					recordXML[pos].endPos = add - 1;
//
//					errorXML = OK;
//					break;
//				}
//			}
//		}
//	}
//	return errorXML;
//}


/*
  *
  *
  *		GetXmlESC( char *record )	Le um registro XML em ESC e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
  *						Salva a posição de inicio e fim do conteúdo a elas associada.
  *
  *		record = informação com o registro XML.
  *		pos = se for ler GetXmlALL vem com a ultima posição lida caso contrário é zero.
  *
  *		retorno = o código do erro.
  *
  */


int GetXmlESC( const char* record,int* pos )
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
				errorXML = CHARNOTSPACEPOINTFEATURE;

				/* para evitar um loop infinito e exibir a TAG errada */
				while( count < _size )
				{
					/* procuro o caracter de fechamento da TAG */
					if( !memcmp( &record[count],"&gt;",4 ) || record[count + 4] == ' ' )
					{
						/* salvo o nome da função */
						memcpy( nameFunctionError,"GetXmlESC",strlen( "GetXmlESC" ) );
						break;
					}
					/* salvo o valor lido */
					tagError[count2++] = record[count++];
				}

				return errorXML;
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
					errorXML = CHARNOTALLOWED;

					/* salvo o nome da função */
					memcpy( nameFunctionError,"GetXmlESC",strlen( "GetXmlESC" ) );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags].nameTAG,strlen( recordXML[amountTags].nameTAG ) );
					return errorXML;
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
	return errorXML;
}

/*
 *
 *
 *		GetXmlALL( char *record )	Le um registro XML com conteúdo ESC e FIX e analisa a existencia da TAG de abertura e fechamento e salvando o nome se identicas.
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
				errorXML = CHARNOTSPACEPOINTFEATURE;

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

				return errorXML;
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
					errorXML = CHARNOTALLOWED;

					/* salvo o nome da função */
					memcpy( nameFunctionError,"GetXmlALL",strlen( "GetXmlALL" ) );

					/* limpo a tagError*/
					memset( tagError,0x00,TAGERROR );

					/* salvo o nome da TAG */
					memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
					return errorXML;
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
			errorXML = TAGNOTEXIST;

			/* procuro ao contrário para diminuir o tempo e a repetição */
			for( int pos = amountTags - 1;pos > -1;pos-- )
			{
				/* procuro a TAG de fechamento */
				if( !memcmp( nameEndTag,recordXML[pos].nameTAG,( strlen( nameEndTag ) ) &&
					strlen( nameEndTag ) == strlen( recordXML[pos].nameTAG ) ) && !recordXML[pos].endPos )
				{
					/* salvo a posição inicial */
					recordXML[pos].endPos = add - 1;

					errorXML = OK;
					break;
				}
			}
			/* se houve erro não continuo */
			if( errorXML != OK )
			{
				/* salvo o nome da função */
				memcpy( nameFunctionError,"GetXmlALL",strlen( "GetXmlALL" ) );

				/* limpo a tagError*/
				memset( tagError,0x00,TAGERROR );

				/* salvo o nome da TAG */
				memcpy( tagError,recordXML[amountTags - 1].nameTAG,strlen( recordXML[amountTags - 1].nameTAG ) );
				return errorXML;
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
			GetXmlESC( record,&count );
			typeFRAME = ESCFRAME; /* ajusto para o ESC*/
		}

		if( !memcmp( &record[count],"8=",2 ) && record[count - 1] != 0x01 )
		{
			ReleaseMemoryFIX();
			GetFix( record,varNew,&count );
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

	return errorXML;
}
