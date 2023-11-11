// TrataXML.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include "VerifyGetXML.h"
#include "GetFIX.h"

#include <stdio.h>
#include <time.h>
#include <chrono>

int main()
{
	using namespace std::chrono;

	char* buffer = NULL;
	long tamanho = 0; // tamanho em bytes do arquivo
	FILE* archXML = NULL; /* crio o ponteiro de endereço do arquivo que será aberto */
	errno_t errorXML = NULL; /* erro retornado da abertura do arquivo */

	/* abro o arquivo somente pra leitura e verifico se o arquivo foi aberto*/
	errorXML = fopen_s( &archXML,"D:/Projetos/C++/XMLFIX.xml","rb" );
	if( errorXML == NULL )
	{
		fseek( archXML,0,SEEK_END );

		// pega a posição corrente de leitura no arquivo
		tamanho = ftell( archXML );

		/* retorno o ponteiro para o inicio do arquivo */
		fseek( archXML,0,SEEK_SET );

		/* reservo a memória */
		buffer = (char*)calloc( tamanho + 1,sizeof( char* ) );
		if( buffer != NULL )
		{
			/* Le o registro todo do arquivo */
			fread( buffer,sizeof( char ),tamanho,archXML );

			// Start measuring time
			//clock_t start = clock();
			auto t1 = high_resolution_clock::now();

			int val = GetXmlALL( buffer,true );

			if( val != OK )
			{
				memcpy( buffer,GetError(),strlen( GetError() ) + 1 );
				printf( "%s.\n",buffer );
			}
			else
			{
				char* content = NULL;
				int size = 0;
				//printf( "%s\n",GetTag( buffer,(char*)"Header",true,&size,false ) );
				//printf( "%s\n",GetTag( buffer,(char*)"Action",true,&size,true ) );
				//printf( "%s\n",GetTag( buffer,(char*)"messageData",true,&size,false ) );
				//printf( "%s\n",GetTag( buffer,(char*)"BizMsgIdr",true,&size,false ) );
				//printf( "%s\n",GetTag( buffer,(char*)"GvUpOnCaptr",true,&size,false ) );
				//printf( "%s\n",GetTag( buffer,(char*)"participant",true,&size,false,(char*)"code" ) );
				//printf( "%s\n",GetTag( buffer,(char*)"routingInformation",true,&size,false,(char*)"code" ) );

				GetTag( buffer,(char*)"Header",true,&size,false );
				GetTag( buffer,(char*)"Action",true,&size,true );
				GetTag( buffer,(char*)"messageData",true,&size,false );
				GetTag( buffer,(char*)"GvUpOnCaptr",true,&size,false );
				GetTag( buffer,(char*)"participant",true,&size,false,(char*)"code" );
				GetTag( buffer,(char*)"routingInformation",true,&size,false,(char*)"code" );
				//printf( "%s\n",buffer );

				//	GetTag( buffer,(char*)"soap:Header",false );

				//GetTag( buffer,(char*)"ns2:messageData",false,&size );
				//content = GetTag( buffer,(char*)"ns2:messageData",false,&size );
				//int valFIX = GetFix( content,&size );
				//printf( "%s\n",GetNameFIX( (char*)"35",true ) );
				//printf( "%s\n",GetNameFIX( (char*)"35553",true ) );
				//printf( "%s\n",GetNameFIX( (char*)"37",true ) );
				//printf( "%s\n",GetNameFIX( (char*)"524",true ) );

				GetNameFIX( (char*)"35",true,true );
				GetNameFIX( (char*)"35553",true,false );
				GetNameFIX( (char*)"37",true,false );
				GetNameFIX( (char*)"524",true,false );

				//val = GetXmlHTML( content = GetTag( buffer,(char*)"ns2:messageData",false,&size ),&val );
				//GetTag( buffer,(char*)"ns2:participant",false,&size,(char*)"ns2:code" );
				//GetTag( buffer,(char*)"ns2:routingInformation",false,&size,(char*)"ns2:code" );
				//GetTag( buffer,(char*)"ClrAcct",true,&size );
				//GetTag( buffer,(char*)"TradLegDtls",true,&size,(char*)"TradDt" );
				//GetTag( buffer,(char*)"TradgPty",true,&size,(char*)"Id" );

				//GetTag( content,(char*)"PrtryId",false,&size,(char*)"Id" );
				//printf( "%s\n",GetTag( content,(char*)"PrtryId",false,&size,(char*)"Id" ) );
				//	printf( "%s\n",GetTag( buffer,(char*)"ClrAcct",true,size,(char*)"Id" ) );
				//	printf( "%s\n",GetTag( buffer,(char*)"TradLegDtls",true,size,(char*)"TradDt" ) );
				//	printf( "%s\n",GetTag( buffer,(char*)"TradgPty",true,size,(char*)"Id" ) );


				if( error != OK || flagError == true )
				{
					memcpy( buffer,GetError(),strlen( GetError() ) + 1 );
					printf( "%s.\n",buffer );
				}

				if( errorFIX != OK || flagErrorFIX == true )
				{
					memcpy( buffer,GetErrorFIX(),strlen( GetErrorFIX() ) + 1 );
					printf( "%s.\n",buffer );
				}
				free( content );
			}
			auto t2 = high_resolution_clock::now();

			//Stop measuring time and calculate the elapsed time
			//duration<double> time_span = duration_cast<duration<double>>( t2 - t1 );
			auto duration = duration_cast<microseconds>( t2 - t1 );

			std::cout << "Tempo gasto :" << (double)( duration.count() / 1000.0 ) << " Micro Segundos.";
			std::cout << std::endl;

			/* libero o ponteiro,libero e fecho o arquivo */
			fflush( archXML );
			free( buffer );
			fclose( archXML );

			///* libero a struct e seus componentes do XML */
			//ReleaseMemory();
		}
		else
			printf( "Nao e possível reservar memoria \n" );

	}
	else
		printf( "Arquivo inexistente ERRO: %d\n",errorFIX );

	return 0;
}

