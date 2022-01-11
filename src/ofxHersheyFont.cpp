/*
 *  ofxHersheyFont.cpp
 *
 *  Created by Tobias Zimmer, August 2016.
 *  www.tobiaszimmer.net
 *
 *  Font originally developed by Dr. Allen V. Hershey in 1967.
 *  Font vectors made available by Paul Bourke.
 *  paulbourke.net/dataformats/hershey/
 *  
 *  A simple single line font for machines like CNC, Lasercutter, ...
 *  Available characters: ASCII codes 32 to 126.
 *
 */

#include "ofxHersheyFont.h"

//--------------------------------------------------------------
ofxHersheyFont::ofxHersheyFont(){
    color = ofColor(0);
}

void ofxHersheyFont::loadSVGFont(filesystem::path _fontPath){
 
    fontPath = _fontPath;
    fontName = fontPath.stem().string();
    
    //    Poco::Timestamp fileTime = Poco::File(file.string()).getLastModified();
    //    ofLog()<<file.filename().string()<<" : "<<print_pretty_datetime(fileTime);
    //        ofLog()<<"isDirectory "<<Poco::File(file.string()).isDirectory();
    //        ofLog()<<"root_name "<<file.root_name().string();
    //        ofLog()<<"stem "<<file.stem().string();
    //        ofLog()<<"extension "<<file.extension().string();
    // file.parent
    
    if( svgFontFile.load(fontPath) ){
        ofLog()<<"loaded "<<fontPath;
    }else{
        ofLog()<<"unable to load "<<fontPath<<" check data/ folder";
    }
    
}

//--------------------------------------------------------------
void ofxHersheyFont::draw(string stringValue, float xPos, float yPos, float scale, bool centered, float angle) {
    
    float characterXPos = 0;
    float center = 0;
    if (centered) center = getWidth(stringValue, scale) / 2;
    
    ofPushMatrix();
    ofTranslate(xPos, yPos);
    ofRotateDeg(angle);
    ofTranslate(-center, 0);
    
    ofPath charPath = getPath(stringValue,scale);
    charPath.draw();
   
    ofPopMatrix();
}
//--------------------------------------------------------------
void ofxHersheyFont::drawSpecial(string stringValue, float xPos, float yPos, float scale, bool centered, float angle) {
    
    float characterXPos = 0;
    float center = 0;
    if (centered) center = getWidth(stringValue, scale) / 2;
    
    ofPushMatrix();
    ofTranslate(xPos, yPos);
    ofRotateDeg(angle);
    ofTranslate(-center, 0);
    
    //getPathSpecial(string stringValue, float x, float y, float scale, int yFlip)
    ofPath charPath = getPathSpecial(stringValue,0,0,scale,-1);
    charPath.draw();
   
    ofPopMatrix();
}

//--------------------------------------------------------------
void ofxHersheyFont::setColor(ofColor c) {
    color = c;
}

//--------------------------------------------------------------
ofRectangle ofxHersheyFont::getBoundingBox(string stringValue, float scale, glm::vec2 pos){
    
    if(currentPhrase != stringValue || boundingBox.getPosition() != glm::vec3(pos.x,pos.y,0) || currentScale != scale){
        currentPhrase = stringValue;
        currentScale = scale;
        
        ofLog()<<"getBoundingBox() scale:"<<scale;
        
        float stringWidth = 0;
        float stringHeight = 0;
        float lineHeight = getLineHeight(scale);
        
        vector<string> stringValue_split = ofSplitString(stringValue, delimiter);
        
        for(auto & aString : stringValue_split){
            
            float temp_width = 0;
            ofLog()<<"aString "<<aString;
            for (int i = 0; i < aString.size(); i++)
            {
                string nextChar = ofToString(aString.at(i));
                string elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
                
                if(svgFontFile.findFirst(elementPath) == 0 ){
                    ofLog()<<"char "<<nextChar<<" not in svg replace with question mark";
                    nextChar = "?";
                    elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
                }
                
                ofXml xmlElement = svgFontFile.findFirst(elementPath);
                
                float charWidth = ofToFloat(xmlElement.getAttribute("horiz-adv-x").getValue());
                temp_width += charWidth * scale;
            }
            stringHeight += lineHeight;
            if(temp_width > stringWidth){
                stringWidth = temp_width;
            }
        }
        
        boundingBox = ofRectangle(pos.x, pos.y, stringWidth, stringHeight);
        ofLog()<<"boundingBox "<<boundingBox;
    }
    
    return boundingBox;
}
//--------------------------------------------------------------
float ofxHersheyFont::getWidth(string stringValue,float scale) {
    return getBoundingBox(stringValue,scale).getWidth();
}
//--------------------------------------------------------------
float ofxHersheyFont::getHeight(string stringValue,float scale) {
    return getBoundingBox(stringValue,scale).getHeight();
}
//--------------------------------------------------------------
float ofxHersheyFont::getLineHeight(float scale) {
    //the height of a capital letter is 21px (scale 1)
    
//    ofLog()<<"ofxHersheyFont::getHeight()";
    
    ofPath charPath = getPath("I",scale);
//    ofLog()<<"charPath.getOutline() "<<charPath.getOutline().size();
    ofPolyline charPolyline;
    for(auto & aLine : charPath.getOutline()){
        charPolyline.addVertices(aLine.getVertices());
    }
    float stringHeight = charPolyline.getBoundingBox().getHeight();
    
    return stringHeight;
}

//--------------------------------------------------------------
ofPath ofxHersheyFont::getPath(string stringValue, float x, float y, float scale, int yFlip){

    ofPath charPath;
    
    //iterate through each character of the input string
    for (int i = 0; i < stringValue.size(); i++)
    {
    
        string nextChar = ofToString(stringValue.at(i));
        string elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
        
        if(svgFontFile.findFirst(elementPath) == 0 ){
            ofLog()<<"char "<<nextChar<<" not in svg replace with question mark";
            nextChar = "?";
            elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
        }
        
        ofXml xmlElement = svgFontFile.findFirst(elementPath);
        
        float charWidth = ofToFloat(xmlElement.getAttribute("horiz-adv-x").getValue());
        
        vector<string> splitGlyphPath = ofSplitString(xmlElement.getAttribute("d").getValue(), " ");//glyph path data in SVG looks like this: "M 139 -9.45 L 230 18.9 L 299 22.1 L 227 25.2"
        
        for(int i=0; i<splitGlyphPath.size(); i+=3){
            if(splitGlyphPath[i] == "M"){
                charPath.moveTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
            }else if(splitGlyphPath[i] == "L"){
                charPath.lineTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
            }
        }
        
        x += (charWidth*scale);
    }
    //move to first coordinate
//    chPath.moveTo(simplex[asciiValue - 32][2], simplex[asciiValue - 32][3]);

    charPath.setStrokeColor(color);
    charPath.setStrokeWidth(1);
    charPath.setFilled(false);
    
    return charPath;
}

//-----------------------
ofPath ofxHersheyFont::getPathSpecial(string stringValue, float x, float y, float scale, int yFlip){

//    ofLog()<<"getPath scale "<<scale<<" _resampleSpacing "<<_resampleSpacing;

    float start_x = x;
    bool singlePath = true;
    
    ofPath charPath;
    
    glm::vec2 lastPoint(x,y);
    
    //iterate through each character of the input string
    for (int i = 0; i < stringValue.size(); i++)
    {
    
        string nextChar = ofToString(stringValue.at(i));
        
        bool firstLetterInWord = false;
        if(i == 0){
            firstLetterInWord = true;
        }else{
            string prevChar = ofToString(stringValue.at(i-1));
            if(prevChar == "/" || prevChar == " ") firstLetterInWord = true;
        } 
        if(nextChar == "/"){
            y += getLineHeight(scale);
            x = start_x;
            continue;
        }
        string elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
        
        if(svgFontFile.findFirst(elementPath) == 0 ){
            ofLog()<<"char "<<nextChar<<" not in svg replace with question mark";
            nextChar = "?";
            elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
        }
        
        ofXml xmlElement = svgFontFile.findFirst(elementPath);
        
        float charWidth = ofToFloat(xmlElement.getAttribute("horiz-adv-x").getValue());
        
        vector<string> splitGlyphPath = ofSplitString(xmlElement.getAttribute("d").getValue(), " ");//glyph path data in SVG looks like this: "M 139 -9.45 L 230 18.9 L 299 22.1 L 227 25.2"
        
        if(splitGlyphPath.size() > 1){
            
           
            for(int i=0; i<splitGlyphPath.size(); i+=3){
                
                if(firstLetterInWord == true){
                    firstLetterInWord = false;
                    charPath.moveTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                } else if(singlePath == true && i == 0 && firstLetterInWord == false){
                    glm::vec2 newPoint = glm::vec2(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                    if(distance(lastPoint,newPoint) > (charWidth*scale*0.5)){
                        charPath.moveTo(newPoint.x,newPoint.y);
                    } else {
                        charPath.lineTo(newPoint.x,newPoint.y);
                    }
                    //                    charPath.lineTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                }else{
                    if(splitGlyphPath[i] == "M"){
                        charPath.moveTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                    }else if(splitGlyphPath[i] == "L"){
                        charPath.lineTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                    }
                }
                
                lastPoint = glm::vec2(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
            }
        }
        x += (charWidth*scale);
    }
    //move to first coordinate
    //    chPath.moveTo(simplex[asciiValue - 32][2], simplex[asciiValue - 32][3]);
    
    
    charPath.setStrokeColor(color);
    charPath.setStrokeWidth(1);
    charPath.setFilled(false);
    
    if(smoothingSize == 0 && smoothingSize == 0){
        return charPath;
    } else {
        
        vector<ofPolyline> charPolyline = charPath.getOutline();
        
        ofPath charPath_resampled;
        //    ofLog()<<"charPolyline "<<charPolyline.size();
        //    ofLog()<<"_resampleSpacing "<<_resampleSpacing;
        for(auto & aLine : charPolyline){
            
            if(resampleSpacing > 0){
                aLine = aLine.getResampledBySpacing(resampleSpacing);
            }
            if(smoothingSize > 0){
                aLine = aLine.getSmoothed(smoothingSize,smoothingShape);
            } 
            //        ofLog()<<"temp_pLine "<<temp_pLine.size();
            for(int i=0; i<aLine.getVertices().size(); i++){
                if(i == 0) charPath_resampled.moveTo(aLine.getVertices()[i]);
                else charPath_resampled.lineTo(aLine.getVertices()[i]);
            }
        }
        
        charPath_resampled.setStrokeColor(color);
        charPath_resampled.setStrokeWidth(1);
        charPath_resampled.setFilled(false);
        
        return charPath_resampled;
    }
}
//--------------------------------------------------------------
string ofxHersheyFont::getPath_asPythonString(string stringValue, string prefix, float x, float y, float scale, int yFlip) {
    
    string str_path = "";
    ofLog()<<"getPath_asPythonStringResampled()";
    
    ofPath charPath = getPath(stringValue,0,0,scale,yFlip);
    vector<ofPolyline> charPolyline = charPath.getOutline();
    
    for(auto & aLine : charPolyline){
        
        for(int i=0; i<aLine.getVertices().size(); i++){
//            if(i == 0) charPath_resampled.moveTo(temp_pLine.getVertices()[i]);
//            else charPath_resampled.lineTo(temp_pLine.getVertices()[i]);
            glm::vec3 v = aLine.getVertices()[i];
            str_path += prefix;
            if(i == 0){
                str_path += "moveto(";
                str_path += ofToString(x + v.x,2);
                str_path += ",";
                str_path += ofToString(y + v.y,2);
                str_path += ")\n";
            }else {       
                str_path += "lineto(";
                str_path += ofToString(x + v.x,2);
                str_path += ",";
                str_path += ofToString(y + v.y,2);
                str_path += ")\n";
            }
            
        }
    }

    return str_path;
}

