import 'dart:io';

final HOST = "127.0.0.1"; // eg: localhost 
final PORT = 5000; 

void main() {
  HttpServer.bind(HOST, PORT).then((server) {
    server.listen((HttpRequest request) {
      switch (request.method) {
        case "GET": 
          handleGet(request);
          break;
        case "POST": 
          handlePost(request);
          break;
        case "OPTIONS": 
          handleOptions(request);
          break;
        default: defaultHandler(request);
      }
    }, 
    onError: printError);
    
    print("Listening for GET and POST on http://$HOST:$PORT");
  },
  onError: printError);
}

/**
 * Handle GET requests by reading the contents of data.json
 * and returning it to the client
 */
void handleGet(HttpRequest req) {
  HttpResponse resp = req.response;
  print("${req.method}: ${req.uri.path}");
  addCorsHeaders(resp);
  
  // remove the leading '/'
  String path = req.uri.path.substring(1);
  // read the contents and stream out port
  var file = new File(path);
  String contenttype;
  if( path.contains('.html') )
    contenttype = "text/html";
  else if( path.contains('.json') )
    contenttype = "application/json";
  else if( path.contains('.dart') )
    contenttype = "application/dart";
  else if( path.contains('.js') )
    contenttype = "application/javascript";
  else if( path.contains('.css') )
    contenttype = "text/css";
  else if( path.contains('.png') )
    contenttype = "image/png";
  else  
    contenttype = "text";
    
  if (file.existsSync()) {
    resp.headers.add(HttpHeaders.CONTENT_TYPE, contenttype);
    file.readAsBytes().asStream().pipe(resp); // automatically close output stream
  }
  else {
    var err = "Could not find file: ${req.uri.path}";
    resp.addError(err);
      //stderr.writeln(err);
      resp.close().catchError(printError);
  }
  
}

File    file;
IOSink  ioSink; // save the data to the file

/**
 * Handle POST requests by overwriting the contents of data.json
 * Return the same set of data back to the client.
 */
void handlePost(HttpRequest req) {
  HttpResponse res = req.response;
  print("${req.method}: ${req.uri.path}");
  
  addCorsHeaders(res);
  // remove the leading '/'
  String path = req.uri.path.substring(1);
  if (path=="") path="untitled"; 
  file = new File(path);
  ioSink = file.openWrite(); // save the data to the file
  
  req.listen((List<int> buffer) {
    ioSink.add(buffer);

    print("buffer size=${buffer.length};");
    
    // return the same results back to the client
    res.add(buffer);
    res.close();
  },
    onDone: () { ioSink.close(); }, onError: printError);
}

/**
 * Add Cross-site headers to enable accessing this server from pages
 * not served by this server
 * 
 * See: http://www.html5rocks.com/en/tutorials/cors/ 
 * and http://enable-cors.org/server.html
 */
void addCorsHeaders(HttpResponse res) {
  res.headers.add("Access-Control-Allow-Origin", "*, ");
  res.headers.add("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
  res.headers.add("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
}

void handleOptions(HttpRequest req) {
  HttpResponse res = req.response;
  addCorsHeaders(res);
  //print("${req.method}: ${req.uri.path}");
  res.statusCode = HttpStatus.NO_CONTENT;
  res.close();
}

void defaultHandler(HttpRequest req) {
  HttpResponse res = req.response;
  addCorsHeaders(res);
  res.statusCode = HttpStatus.NOT_FOUND;
  res.addError("Not found: ${req.method}, ${req.uri.path}");
  res.close();
}

void printError(error) => print(error);
