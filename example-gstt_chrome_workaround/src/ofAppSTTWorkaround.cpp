#include "ofAppSTTWorkaround.h"

//--------------------------------------------------------------
void ofAppSTTWorkaround::setup(){
    // setup a server with default options on port 9092
    // - pass in true after port to set up with SSL
    //bSetup = server.setup( 9093 );

    ofxLibwebsockets::ServerOptions options = ofxLibwebsockets::defaultServerOptions();
    options.port = 9092;
	options.bUseSSL = false; // you'll have to manually accept this self-signed cert if 'true'!
    bSetup = server.setup( options );

    // this adds your app as a listener for the server
    server.addListener(this);

    ofBackground(0);
    ofSetFrameRate(60);
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::update(){
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::draw(){
    if ( bSetup ){
        ofDrawBitmapString("WebSocket server setup at "+ofToString( server.getPort() ) + ( server.usingSSL() ? " with SSL" : " without SSL"), 20, 20);

        ofSetColor(150);
        ofDrawBitmapString("Click anywhere to open up client example", 20, 40);
    } else {
        ofDrawBitmapString("WebSocket setup failed :(", 20,20);
    }

    int x = 20;
    int y = 100;

    ofSetColor(0,150,0);
    ofDrawBitmapString("Console", x, 80);

    ofSetColor(255);
    for (int i = messages.size() -1; i >= 0; i-- ){
        ofDrawBitmapString( messages[i], x, y );
        y += 20;
    }
    if (messages.size() > NUM_MESSAGES) messages.erase( messages.begin() );

    ofSetColor(150,0,0);
    ofDrawBitmapString("Type a message, hit [RETURN] to send:", x, ofGetHeight()-60);
    ofSetColor(255);
    ofDrawBitmapString(toSend, x, ofGetHeight() - 40);
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::onConnect( ofxLibwebsockets::Event& args ){
    cout<<"on connected"<<endl;
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::onOpen( ofxLibwebsockets::Event& args ){
    cout<<"new connection open"<<endl;
    messages.push_back("New connection from " + args.conn.getClientIP() + ", " + args.conn.getClientName() );
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::onClose( ofxLibwebsockets::Event& args ){
    cout<<"on close"<<endl;
    messages.push_back("Connection closed");
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::onIdle( ofxLibwebsockets::Event& args ){
    cout<<"on idle"<<endl;
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::onMessage( ofxLibwebsockets::Event& args ){
    cout<<"got message "<<args.message<<endl;

    // trace out string messages or JSON messages!
    if ( !args.json.isNull() ){
        messages.push_back("New message: " + args.json.toStyledString() + " from " + args.conn.getClientName() );
    } else {
        messages.push_back("New message: " + args.message + " from " + args.conn.getClientName() );
    }

    // echo server = send message right back!
    args.conn.send( args.message );
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::onBroadcast( ofxLibwebsockets::Event& args ){
    cout<<"got broadcast "<<args.message<<endl;
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::keyPressed(int key){
    // do some typing!
    if ( key != OF_KEY_RETURN ){
        if ( key == OF_KEY_BACKSPACE ){
            if ( toSend.length() > 0 ){
                toSend.erase(toSend.end()-1);
            }
        } else {
            toSend += key;
        }
    } else {
        // send to all clients
        server.send( toSend );
        messages.push_back("Sent: '" + toSend + "' to "+ ofToString(server.getConnections().size())+" websockets" );
        toSend = "";
    }
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::keyReleased(int key){

}

//--------------------------------------------------------------
void ofAppSTTWorkaround::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofAppSTTWorkaround::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofAppSTTWorkaround::mousePressed(int x, int y, int button){
    string url = "http";
    if ( server.usingSSL() ){
        url += "s";
    }
    url += "://localhost:" + ofToString( server.getPort() );
    ofLaunchBrowser(url);
}

//--------------------------------------------------------------
void ofAppSTTWorkaround::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofAppSTTWorkaround::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofAppSTTWorkaround::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofAppSTTWorkaround::dragEvent(ofDragInfo dragInfo){

}
