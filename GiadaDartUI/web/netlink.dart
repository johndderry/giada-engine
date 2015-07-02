library NetLink;

import 'dart:html';

const  HOSTNAME  = 'localhost:5000';

class HtmlSendLink {
  
  HttpRequest req;
  
  String message, statmess;
  
  void HtmlSendLink() {
    req = new HttpRequest();
    req.onReadyStateChange.listen(ReadyStateChange);
  }
  
  void sendMessage(String UrlPath, String message) {
    
    req.open('POST', 'http://${HOSTNAME}/${UrlPath}', async:false);
    if( message == null || message.length == 0 ) 
      req.send( " " );
    else
      req.send( message );
  
  }
  
  void ReadyStateChange( Event pe ) 
  {
    if (req.readyState == HttpRequest.DONE &&
         (req.status == 200 || req.status == 0))
       statmess = 'post success';
     else 
       statmess = 'post failure: ${pe.toString()}';
  }
}