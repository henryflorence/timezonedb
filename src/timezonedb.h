#ifndef  timezonedb_h
#define  timezonedb_h
//#define debug true

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

const char* server = "api.timezonedb.com";
const char* timezonedbGetURLFormat = "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s&fields=gmtOffset";
const char* timezonedbGetTimezoneInfoURLFormat = "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s";
const char* timezonedbGetTimezoneDstURLFormat = "http://api.timezonedb.com/v2/get-time-zone?key=%s&format=json&by=zone&zone=%s&fields=dst,dstStart,dstEnd,gmtOffset";

//char* timezonedbAPIkey = "---your api key---";
//char* timezonedbLocation = "Europe/London";

char getURL[ 115 ];
const unsigned long HTTP_TIMEOUT = 10000;
const size_t MAX_CONTENT_SIZE = 512;



struct UserData
{
  char status   [ 32 ];
  char message  [ 32 ];
  char gmtOffset[ 32 ];
};

// Open connection to the HTTP server
bool httpConnect( const char* hostName )
{
#ifdef debug
  Serial.print( "HTTP connection to : " );
  Serial.print( hostName );

  long T1 = millis();
#endif
  bool httpStatus = client.connect( hostName, 80 );
#ifdef debug
  Serial.print  ( httpStatus ? " / OK" : " / Error" );
  Serial.print  ( " / dT = " );
  Serial.print  ( millis() - T1 );
  Serial.println( " ms " );
#endif
  return httpStatus;
}

// Send the HTTP GET request to the server
bool sendRequest( const char* host, const char* getURL )
{
#ifdef debug
  Serial.print( "GET :                       " );
#endif

  client.print( "GET " );
  client.print( getURL );
  client.print( " HTTP/1.1\r\nHost: " );
  client.print( server );
  client.print( "\r\nConnection: close\r\n\r\n" );

#ifdef debug
  Serial.println( getURL );
#endif

  return true;
}

// Close the connection with the HTTP server
void httpDisconnect()
{
  // Serial.println( "Disconnect" );
  client.stop();
}

// Skip HTTP headers so that we are at the beginning of the response's body
bool skipResponseHeaders()
{
  // HTTP headers end with an empty line
  char endOfHeaders[] = "\r\n\r\n";

  client.setTimeout( HTTP_TIMEOUT );
  bool ok = client.find( endOfHeaders );

#ifdef debug
  Serial.print( "HTTP response :             " );
  if( !ok )
    Serial.println( "Invalid response" );
  else
    Serial.println( "OK" );
#endif

  return ok;
}


// Read the body of the response from the HTTP server
void readReponseContent( char* content, size_t maxSize )
{
#ifdef debug
  long T1 = millis();
  Serial.print( "Client read time :          " );
#endif
  size_t length = 0;
  while( client.available() )
    content[ length++ ] = client.read();
  content[ length ] = 0;
#ifdef debug
  long dT = millis() - T1;
  Serial.println( dT );
  Serial.print( "No of bytes             " );
  Serial.println( length );
#endif

  int contentSize = ( int )strlen( content );

  char * pch;

  int msgStart = 0;
  pch = strchr( content, '{' );
  msgStart = pch-content+1;

  int msgStop = 0;
  pch = strrchr( content, '}' );
  msgStop = pch-content+1;

  for( int i=0; i<msgStop; i++ )
    { content[ i ] = content[ i + msgStart - 1 ] ; }
  for( int i=msgStop-3; i<=contentSize; i++ )
    { content[ i ] = '\0' ; }

#ifdef debug 
  Serial.print( "Response from server :        " );
  Serial.println( content );
#endif
}


bool parseUserData( char* content, struct UserData* userData )
{
  // Compute optimal size of the JSON buffer according to what we need to parse.
  // This is only required if you use StaticJsonBuffer.
  //const size_t BUFFER_SIZE =
  //      JSON_OBJECT_SIZE( 3 );

  // Allocate a temporary memory pool on the stack
  //StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  // If the memory pool is too big for the stack, use this instead:
  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject( content );

  if (!root.success())
  {
#ifdef debug
    Serial.println( "JSON parsing failed!" );
#endif
    return false;
  }

  strcpy( userData->status,    root[ "status"    ] );
  strcpy( userData->message,   root[ "message"   ] );
  strcpy( userData->gmtOffset, root[ "gmtOffset" ] );
  // It's not mandatory to make a copy, you could just use the pointers
  // Since, they are pointing inside the "content" buffer, so you need to make
  // sure it's still in memory when you read the string

  return true;
}

// Print the data extracted from the JSON
void printUserData( const struct UserData* userData )
{
  Serial.print  ( "status = " );
  Serial.println( userData->status );
  Serial.print  ( "message = " );
  Serial.println( userData->message );
  Serial.print  ( "gmtOffset = " );
  Serial.println( userData->gmtOffset );
  long l_gmtOffset = atoi( userData->gmtOffset );
  Serial.print  ( "l_gmtOffset = " );
  Serial.println( l_gmtOffset );
}



void getTimezone( long *gmtOffset )
{
  if( ! httpConnect( server ) )
    return;

  sprintf( getURL, timezonedbGetURLFormat, timezonedbAPIkey, timezonedbLocation );
  if( sendRequest( server, getURL ) && skipResponseHeaders() )
  {
    char response[ MAX_CONTENT_SIZE ];
    readReponseContent( response, MAX_CONTENT_SIZE );

    UserData userData;
    if( parseUserData( response, &userData ) )
    {
      // printUserData( &userData );
      *gmtOffset = atol( userData.gmtOffset );
    }
  }
  httpDisconnect();
}

void getTimezoneInfo() 
{
  if( ! httpConnect( server ) )
    return;

  sprintf( getURL, timezonedbGetTimezoneInfoURLFormat, timezonedbAPIkey, timezonedbLocation );
  if( sendRequest( server, getURL ) && skipResponseHeaders() )
  {
    char response[ MAX_CONTENT_SIZE ];
    readReponseContent( response, MAX_CONTENT_SIZE );

    UserData userData;
    if( parseUserData( response, &userData ) )
    {
      //printUserData( &userData );
      //*gmtOffset = atol( userData.gmtOffset );
    }
  }
  httpDisconnect();
}

boolean getTimezoneDst(boolean *dst, long *dstStart, long *dstEnd, long *gmtOffset) 
{
  if( ! httpConnect( server ) )
    return false;

  sprintf( getURL, timezonedbGetTimezoneDstURLFormat, timezonedbAPIkey, timezonedbLocation );
  if( sendRequest( server, getURL ) && skipResponseHeaders() )
  {
    char response[ MAX_CONTENT_SIZE ];
    readReponseContent( response, MAX_CONTENT_SIZE );

    DynamicJsonBuffer jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject( response );

    if (!root.success())
    {
#ifdef debug
      Serial.println( "JSON parsing failed!" );
#endif
      httpDisconnect();
      return false;
    }

    *dst =              atol( root[ "dst"      ] );
    *dstStart =         atol( root[ "dstStart" ] );
    *dstEnd =           atol( root[ "dstEnd"   ] );
    *gmtOffset =        atol( root[ "gmtOffset"] );

  }
  httpDisconnect();
  return true;
}
#endif // timezonedb_h
