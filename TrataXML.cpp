// TrataXML.cpp : Este arquivo contém a função 'main'. A execução do programa começa e termina ali.
//

#include <iostream>
#include "VerifyGetXML.h"
#include "GetFIX.h"

#include <stdio.h>
#include <time.h>
#include <chrono>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream> 
#include <shared_mutex> 

using namespace std;
std::mutex	mutexXML;
shared_mutex mutx;
class teste
{
	public:
	std::mutex _mutex_teste1;
	teste()
	{
	}
	~teste()
	{
	}

	void criavalXML( int num )
	{
		using namespace std::chrono;
		char* buffer = NULL;
		long tamanho = 0; // tamanho em bytes do arquivo
		FILE* archXML = NULL; /* crio o ponteiro de endereço do arquivo que será aberto */
		errno_t errorXML = NULL; /* erro retornado da abertura do arquivo */

		/* abro o arquivo somente pra leitura e verifico se o arquivo foi aberto*/
		errorXML = fopen_s( &archXML,"D:/Projetos/C++/XMLESCxD.xml","rb" );
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

				int val = GetXml( buffer );
				//int val = GetXmlALL( buffer,false );

				if( val != OK )
				{
					//memcpy( buffer,GetError(),strlen( GetError() ) + 1 );
					printf( "%s.\n",GetError() );
				}
				else
				{
					char* content = NULL;
					size_t size = 0;
					//printf( "%s\n",GetTag( buffer,(char*)"Header",true,&size,false ) );
					//printf( "%s\n",GetTag( buffer,(char*)"Action",true,&size,true ) );
					//printf( "%s\n",GetTag( buffer,(char*)"messageData",true,&size,false ) );
					//printf( "%s\n",GetTag( buffer,(char*)"BizMsgIdr",true,&size,false ) );
					////printf( "%s\n",GetTag( buffer,(char*)"MktIdrCd",true,&size,false ) );
					//printf( "%s\n",GetTag( buffer,(char*)"participant",true,&size,false,(char*)"code" ) );
					//printf( "%s\n",GetTag( buffer,(char*)"routingInformation",true,&size,false,(char*)"code" ) );

					GetTag( buffer,(char*)"Header",true,&size,false );
					GetTag( buffer,(char*)"Action",true,&size,true );
					GetTag( buffer,(char*)"messageData",true,&size,false );
					//GetTag( buffer,(char*)"GvUpOnCaptr",true,&size,false );
					GetTag( buffer,(char*)"participant",true,&size,false,(char*)"code" );
					GetTag( buffer,(char*)"routingInformation",true,&size,false,(char*)"code" );
					//printf( "%s\n",buffer );

					//	GetTag( buffer,(char*)"soap:Header",false );

					//GetTag( buffer,(char*)"ns2:messageData",false,&size );
					//content = GetTag( buffer,(char*)"ns2:messageData",false,&size );
					//int valFIX = GetFix( content,&size );
					//printf( "%s\n",GetNameFIX( (char*)"35",true,true ) );
					//printf( "%s\n",GetNameFIX( (char*)"35553",true,false ) );
					//printf( "%s\n",GetNameFIX( (char*)"37",true,false ) );
					//printf( "%s\n",GetNameFIX( (char*)"524",true,false ) );

					//GetNameFIX( (char*)"35",true,true );
					//GetNameFIX( (char*)"35553",true,false );
					//GetNameFIX( (char*)"37",true,false );
					//GetNameFIX( (char*)"524",true,false );

					//val = GetXmlESC( content = GetTag( buffer,(char*)"ns2:messageData",false,&size ),&val );
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


					//if( errorXML != OK || flagError == true )
					//{
					//	memcpy( buffer,GetError(),strlen( GetError() ) + 1 );
					//	printf( "%s.\n",buffer );
					//}

					//if( numberErrorFIX != OK || flagErrorFIX == true )
					//{
					//	memcpy( buffer,GetErrorFIX(),strlen( GetErrorFIX() ) + 1 );
					//	printf( "%s.\n",buffer );
					//}
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
			printf( "Arquivo inexistente ERRO: %d\n",numberErrorFIX );

	}
};

void criavalXML( int num )
{
	std::lock_guard<std::mutex> guard( mutexXML );
	{

		using namespace std::chrono;
		char* buffer = NULL;
		long tamanho = 0; // tamanho em bytes do arquivo
		FILE* archXML = NULL; /* crio o ponteiro de endereço do arquivo que será aberto */
		errno_t errorXML = NULL; /* erro retornado da abertura do arquivo */
		//printf( "%d\r\n",num );
		/* abro o arquivo somente pra leitura e verifico se o arquivo foi aberto*/
		errorXML = fopen_s( &archXML,"D:/Projetos/C++/XMLESCxD.xml","rb" );
		//	std::string str = "<soap:Envelope xmlns:soap=\"http://www.w3.org/2003/05/soap-envelope\"><soap:Header><Action xmlns=\"http://www.w3.org/2005/08/addressing\">http://schemas.bvmf.com.br/WSDL/ItService/SMP/MessageDispatcherService/v02_1/sendAsync</Action><MessageID xmlns=\"http://www.w3.org/2005/08/addressing\">urn:uuid:667a92b8-1ecf-4e9d-aff7-3cd4968a6533</MessageID><To xmlns=\"http://www.w3.org/2005/08/addressing\">http://apspos20101u.intraservice.corp:8283/axis2/services/MessageDispatcherServiceV2_1</To><ReplyTo xmlns=\"http://www.w3.org/2005/08/addressing\"><Address>http://www.w3.org/2005/08/addressing/anonymous</Address></ReplyTo></soap:Header><soap:Body><MessageControl xmlns=\"http://schemas.bvmf.com.br/ItService/MessageControl/v01.0\" xmlns:ns2=\"http://schemas.bvmf.com.br/WSDL/ItService/SMP/MessageDispatcherService/v02_1\"><callBackEndPoint>http://localhost:9091/mock</callBackEndPoint><messageSequence>1</messageSequence><messageTotal>1</messageTotal><possibleDuplication>false</possibleDuplication><requesterSystem>RTC</requesterSystem><requesterSystemModule>AM</requesterSystemModule><responserSystem>SMP</responserSystem><timeStamp>2022-02-18T06:54:22</timeStamp><transactionId>0101_2245020_2586_41</transactionId></MessageControl><ns2:MessageDispatcherSendAsyncReq xmlns=\"http://schemas.bvmf.com.br/ItService/MessageControl/v01.0\" xmlns:ns2=\"http://schemas.bvmf.com.br/WSDL/ItService/SMP/MessageDispatcherService/v02_1\"><ns2:message><ns2:messageData>8=FIX.4.49=47735=UBL34=149=SMPFLHINT0152=20210607-21:30:00.92556=FIX_OUT_SMPFLH_CAP11=BV21510020220131202201311003160338235560=375=2021060754=135558=11=1658581=40453=3448=114447=D452=36448=1447=D452=7448=COLO0447=D452=5448=1234567822=8207=BVMF1843=10031=10.54381=5055=PETR4336=135556=N2361=3567835552=135557=235553=53576766135554=20210607-21:35:05.91635555=137=B31234567891003=467778960=20210607-21:30:00.92432=1000539=1524=SHE525=D538=3635559=110=10</ns2:messageData><ns2:participant><ns2:code>1</ns2:code></ns2:participant><ns2:informationSystem><ns2:routingInformation><ns2:type><ns2:code>UBL</ns2:code></ns2:type></ns2:routingInformation></ns2:message></ns2:MessageDispatcherSendAsyncReq></soap:Body></soap:Envelope>";

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

				int val = GetXml( buffer );
				//int val = GetXmlALL( buffer,false );

				if( val != OK )
				{
					//memcpy( buffer,GetError(),strlen( GetError() ) + 1 );
					printf( "%s.\n",GetError() );
				}
				else
				{
					char* content = NULL;
					size_t size = 0;
					//printf( "%s\n",GetTag( buffer,(char*)"Header",true,&size,false ) );
					//printf( "%s\n",GetTag( buffer,(char*)"Action",true,&size,true ) );
					//printf( "%s\n",GetTag( buffer,(char*)"messageData",true,&size,false ) );
					//printf( "%s\n",GetTag( buffer,(char*)"BizMsgIdr",true,&size,false ) );
					////printf( "%s\n",GetTag( buffer,(char*)"MktIdrCd",true,&size,false ) );
					//printf( "%s\n",GetTag( buffer,(char*)"participant",true,&size,false,(char*)"code" ) );
					//printf( "%s\n",GetTag( buffer,(char*)"routingInformation",true,&size,false,(char*)"code" ) );

					GetTag( buffer,(char*)"Header",true,&size,false );
					GetTag( buffer,(char*)"Action",true,&size,true );
					GetTag( buffer,(char*)"messageData",true,&size,false );
					//GetTag( buffer,(char*)"GvUpOnCaptr",true,&size,false );
					GetTag( buffer,(char*)"participant",true,&size,false,(char*)"code" );
					GetTag( buffer,(char*)"routingInformation",true,&size,false,(char*)"code" );
					//printf( "%s\n",buffer );

					//	GetTag( buffer,(char*)"soap:Header",false );

					//GetTag( buffer,(char*)"ns2:messageData",false,&size );
					//content = GetTag( buffer,(char*)"ns2:messageData",false,&size );
					//int valFIX = GetFix( content,&size );
					//printf( "%s\n",GetNameFIX( (char*)"35",true,true ) );
					//printf( "%s\n",GetNameFIX( (char*)"35553",true,false ) );
					//printf( "%s\n",GetNameFIX( (char*)"37",true,false ) );
					//printf( "%s\n",GetNameFIX( (char*)"524",true,false ) );

					//GetNameFIX( (char*)"35",true,true );
					//GetNameFIX( (char*)"35553",true,false );
					//GetNameFIX( (char*)"37",true,false );
					//GetNameFIX( (char*)"524",true,false );

					//val = GetXmlESC( content = GetTag( buffer,(char*)"ns2:messageData",false,&size ),&val );
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


					//if( errorXML != OK || flagError == true )
					//{
					//	memcpy( buffer,GetError(),strlen( GetError() ) + 1 );
					//	printf( "%s.\n",buffer );
					//}

					//if( numberErrorFIX != OK || flagErrorFIX == true )
					//{
					//	memcpy( buffer,GetErrorFIX(),strlen( GetErrorFIX() ) + 1 );
					//	printf( "%s.\n",buffer );
					//}
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
			printf( "Arquivo inexistente ERRO: %d\n",numberErrorFIX );
	}
}

void teste2( int i )
{
	teste teste1;
	teste1.criavalXML( i );

}
int main()
{

	std::vector<std::thread> threads;

	for( int i = 0; i < 2000; i++ )
	{
		threads.push_back( std::thread( criavalXML,i ) );
	}

	for( auto& th : threads )
	{
		th.join();
	}
	return 0;
}

#include <iostream>
#include <boost/asio.hpp>

int main()
{
	try
	{
		// Inicializar o contexto I/O
		boost::asio::io_context io_context;

		// Resolver o nome do host e o serviço (porta)
		boost::asio::ip::tcp::resolver resolver( io_context );
		auto endpoints = resolver.resolve( "localhost","8080" );

		// Criar e conectar o socket
		boost::asio::ip::tcp::socket socket( io_context );
		boost::asio::connect( socket,endpoints );

		// Construir a mensagem SOAP
		std::string request = "POST /Teste HTTP/1.1\r\n";
		request += "Host: localhost:8080\r\n";
		request += "Content-Type: text/xml; charset=utf-8\r\n";
		request += "Content-Length: " + std::to_string( xmlData.size() ) + "\r\n";
		request += "\r\n";
		request += "<soapenv:Envelope xmlns:soapenv=\"http://schemas.xmlsoap.org/soap/envelope/\" "
			"xmlns:web=\"http://www.example.com/webservice\">"
			"<soapenv:Header/>"
			"<soapenv:Body>"
			"<web:YourFunction>"
			"<arg1>value1</arg1>"
			"<arg2>value2</arg2>"
			"</web:YourFunction>"
			"</soapenv:Body>"
			"</soapenv:Envelope>";

		// Enviar a mensagem SOAP
		boost::asio::write( socket,boost::asio::buffer( request ) );

		// Ler a resposta do servidor
		boost::asio::streambuf response;
		boost::asio::read_until( socket,response,"</soapenv:Envelope>" );

		// Imprimir a resposta
		std::cout << "Resposta: " << &response;

	} catch( std::exception& e )
	{
		std::cerr << "Erro: " << e.what() << std::endl;
	}

	return 0;
}
