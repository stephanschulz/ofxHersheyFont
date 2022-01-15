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
    
    currentScale = 1;
    currentPhrase = "";
    
    getTallestChar(tallestChar,tallestChar_height);
    
    //calc line height based on letter I
    //some letters go in to the negative because the font aesthetic/design requires it so 
    //that's why we should not use the tallest letter as line height
    
    ofPath charPath = getPath("I",1);
    ofPolyline charPolyline;
    for(auto & aLine : charPath.getOutline()){
        charPolyline.addVertices(aLine.getVertices());
    }
    lineHeight = charPolyline.getBoundingBox().getHeight();
    lineHeight *= 1.4; //https://banc.digital/blog/typography-font-sizes-styles-formats/#:~:text=For%20the%20optimal%20readability%20and,pt%20(180%25)%20maximum%20value.
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

//ofRectangle ofxHersheyFont::getBoundingBox(string stringValue){
//    return boundingBox;
//}
//--------------------------------------------------------------
ofRectangle ofxHersheyFont::getBoundingBox(string stringValue, float scale, glm::vec2 pos){
    
    //|| boundingBox.getPosition() != glm::vec3(pos.x,pos.y,0)
    if(currentPhrase != stringValue  || currentScale != scale){
        currentPhrase = stringValue;
        currentScale = scale;
        
        //        ofLog()<<"getBoundingBox() currentPhrase:"<<currentPhrase<<" stringValue "<<stringValue;
        //        ofLog()<<"getBoundingBox() currentScale:"<<currentScale<<" scale "<<scale;
        //        ofLog()<<"getBoundingBox() boundingBox.getPosition():"<<boundingBox.getPosition()<<" pos "<<pos;
        
        ofPath phrasePath = getPathSpecial(stringValue, pos.x, pos.y, scale);
        vector<ofPolyline> phrasePolylines = phrasePath.getOutline();
        
        for(int i=0; i<phrasePolylines.size(); i++){
            ofRectangle temp_bb;
            temp_bb = phrasePolylines[i].getBoundingBox();
            if(i == 0) boundingBox = temp_bb;
            else boundingBox.growToInclude(temp_bb);
        }
        
        /*
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
         */
        //        ofLog()<<"boundingBox "<<boundingBox;
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
    //some letters go in to the negative because the font aesthetic/design requires it so 
    //that's why we should not use the tallest letter as line height
    return lineHeight * scale;
}

void ofxHersheyFont::getTallestChar(string & _bestChar, float & _bestHeight){
    
    _bestHeight = 0;
    _bestChar = "I";
    string curChar = "";
    
    auto glyphXml = svgFontFile.find("/svg/defs/font/glyph");
    ofLog()<<"getTallestChar() glyphXml "<<glyphXml.size();
    for(auto & xmlElement: glyphXml){
        //        ofLog()<<xmlElement ; //.getValue();
        //        cout<<"unicode "<<xmlElement.getAttribute("unicode").getValue()<<" , "<<endl;
        cout<<xmlElement.getAttribute("unicode").getValue()<<" , "; 
        //        cout<<"glyph-name "<<glyphElement.getAttribute("glyph-name").getValue()<<" "<<endl;
        //        cout<<"horiz-adv-x "<<glyphElement.getAttribute("horiz-adv-x").getValue()<<" "<<endl;
        //        cout<<"d "<<glyphElement.getAttribute("d").getValue()<<" "<<endl;
        
        
        //        string elementPath = "/svg/defs/font/glyph[@unicode='"+curChar+"']";
        
        //        ofXml xmlElement = svgFontFile.findFirst(elementPath);
        
        float charWidth = ofToFloat(xmlElement.getAttribute("horiz-adv-x").getValue());
        
        vector<string> splitGlyphPath = ofSplitString(xmlElement.getAttribute("d").getValue(), " ");//glyph path data in SVG looks like this: "M 139 -9.45 L 230 18.9 L 299 22.1 L 227 25.2"
        
        float minY = INT_MAX;
        float maxY = 0;
        float temp_height = 0;
        if(splitGlyphPath.size() > 2){
            for(int i=0; i<splitGlyphPath.size(); i+=3){
                if(splitGlyphPath[i] == "M" || splitGlyphPath[i] == "L"){
                    //                    minY = glm::min(minY,ofToFloat(splitGlyphPath[i+2]));
                    maxY = glm::max(maxY,ofToFloat(splitGlyphPath[i+2]));
                }
                //        if(splitGlyphPath[i] == "M"){
                //            charPath.moveTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                //        }else if(splitGlyphPath[i] == "L"){
                //            charPath.lineTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                //        }
            }
            temp_height = maxY; // - minY;
            if(temp_height > _bestHeight){
                _bestHeight = temp_height;
                _bestChar = xmlElement.getAttribute("unicode").getValue();
            }
        }
        
    }
    
    ofLog()<<"getTallestChar() _bestChar "<<_bestChar<<" _bestHeight "<<_bestHeight;
}
//--------------------------------------------------------------
ofPath ofxHersheyFont::getPath(string stringValue, float x, float y, float scale, int yFlip){
    
    ofPath charPath;
    
    //iterate through each character of the input string
    for (int i = 0; i < stringValue.size(); i++)
    {
        
        string nextChar = ofUTF8Substring(stringValue, i, 1);  //ofToString(stringValue.at(i));
        if(nextChar.length() == 0) continue; //skip empty char which somehow ö has
        
        string elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
        
        if(svgFontFile.findFirst(elementPath) == 0 ){
            //            ofLog()<<"char "<<nextChar<<" not in svg replace with question mark";
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
    
    //    ofLog()<<"getPathSpecial scale "<<scale;
    //    ofLog()<<"stringValue "<<stringValue<<"| size "<<stringValue.size();
    
    //TODO: center all text lines
    
    bool singlePath = true;
    bool centeredText = true;
    
    vector<string> lines = ofSplitString(stringValue, delimiter) ;
    
    //pre flight to determin each lines width
    float greatestWidth = 0;
    vector<float> lineWidths;
    lineWidths.resize(lines.size());
    if(centeredText == true){
        for (int l = 0; l < lines.size(); l++)
        {
            //loop through each line
            lineWidths[l] = 0;
            
            for (int c = 0; c < lines[l].size(); c++)
            {
                //loop through each letter in that line
                string nextChar = ofUTF8Substring(lines[l], c, 1);
                if(nextChar.length() == 0) continue; //skip empty char which somehow ö has
                
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
                    lineWidths[l] +=(charWidth*scale);
                }
            }//end  for (int c = 0; c < lines[l].size(); c++)
            
            if(lineWidths[l] > greatestWidth){
                greatestWidth = lineWidths[l];
            }
        }//end for (int l = 0; l < lines.size(); l++)
        //    ofLog()<<"greatestWidth "<<greatestWidth;
    }
    
    //-------
    float start_x = x;
    ofPath charPath;
   
//        ofLog()<<"-------------lines.size() "<<lines.size();
    //iterate through each character of the input string
    for (int l = 0; l < lines.size(); l++)
    {
        //loop through each line
        x = start_x;
        if(centeredText) x+= ((greatestWidth - lineWidths[l])/2.0);
        
        //        ofLog()<<"greatestWidth "<<greatestWidth;
        //        ofLog()<<"lineWidths[l] "<<lineWidths[l];
        //        ofLog()<<"g-l "<<((greatestWidth - lineWidths[l])/2.0); 
        glm::vec2 lastPoint(x,y);
        string lastChar = "lineStart";
        for (int c = 0; c < lines[l].size(); c++)
        {
           
            //loop through each letter in that line
            string nextChar = ofUTF8Substring(lines[l], c, 1);
//            ofLog()<<"++++++"<<nextChar<<"++++++++";
            if(nextChar.length() == 0) continue; //skip empty char which somehow ö has
            
            //            ofLog()<<l<<" lines[l].size() "<<lines[l].size()<<" nextChar "<<nextChar;
            
            /*
             "&amp;" glyph-name="ampersand"
             "&apos;" glyph-name="quotesingle" 
             */
            ofXml xmlElement;
            string elementPath;
            if(nextChar == "'" || nextChar == "&"){
                //https://www.codetable.net/unicodecharacters
                //handle special characters that don't work with svgFontFile.findFirst(elementPath)
                auto glyphXml = svgFontFile.find("/svg/defs/font/glyph");
//                ofLog()<<"glyphXml ";
                for(auto & glyphElement: glyphXml){
                    ofLog()<<glyphElement ; //.getValue();
//                    cout<<"unicode "<<glyphElement.getAttribute("unicode").getValue()<<" "<<endl;
//                    cout<<"glyph-name "<<glyphElement.getAttribute("glyph-name").getValue()<<" "<<endl;
//                    cout<<"horiz-adv-x "<<glyphElement.getAttribute("horiz-adv-x").getValue()<<" "<<endl;
//                    cout<<"d "<<glyphElement.getAttribute("d").getValue()<<" "<<endl;
                    if(glyphElement.getAttribute("unicode").getValue() == nextChar){
                        xmlElement = glyphElement;
                        break;
                    }
                }
                
            } else {
                elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
        
                if(svgFontFile.findFirst(elementPath) == 0 ){
                    ofLog()<<"char "<<nextChar<<" not in svg replace with question mark";
                    nextChar = "?";
                    elementPath = "/svg/defs/font/glyph[@unicode='"+nextChar+"']";
                }
                xmlElement = svgFontFile.findFirst(elementPath);
            }

//            ofXml xmlElement = svgFontFile.findFirst(elementPath);
            float charWidth = ofToFloat(xmlElement.getAttribute("horiz-adv-x").getValue());

            
            if(nextChar == " "){
//                ofLog()<<"nextChar == space, charWidth "<<charWidth;
                x += (charWidth*scale);
                lastChar = nextChar;
//                lastPoint = glm::vec2(0,0);
                continue;
            }else{
//                ofLog()<<"use nextChar == "<<nextChar;
            }
            
            string glyphData_str = xmlElement.getAttribute("d").getValue();
            
//            if(glyphData_str.length() == 0){
//                //https://www.codetable.net/unicodecharacters
//                ofLog()<<"try unicode hex code";
//                string charAsHex = "&#x"+ofToHex(nextChar)+";";
//                ofLog()<<"charAsHex:"<<charAsHex;
//                elementPath = "/svg/defs/font/glyph[@unicode='"+charAsHex+"']";
//                xmlElement = svgFontFile.findFirst(elementPath);
//                charWidth = ofToFloat(xmlElement.getAttribute("horiz-adv-x").getValue());
//                glyphData_str = xmlElement.getAttribute("d").getValue();
//            }
            
//            ofLog()<<"nextChar "<<nextChar<<" charWidth "<<charWidth;
//            ofLog()<<"nextChar "<<nextChar<<" int "<<ofToInt(nextChar)<<" hex "<<ofToHex(nextChar); //&#xc7;
//            ofLog()<<"glyphData_str "<<glyphData_str;


            
            vector<string> splitGlyphPath = ofSplitString(glyphData_str, " ");//glyph path data in SVG looks like this: "M 139 -9.45 L 230 18.9 L 299 22.1 L 227 25.2"
            //            ofLog()<<l<<" charWidth "<<charWidth<<" splitGlyphPath "<<splitGlyphPath.size();
//            ofLog()<<"nextChar "<<nextChar<<" splitGlyphPath "<<splitGlyphPath.size()<<":"<<splitGlyphPath[0];
            
            for(int i=0; i<splitGlyphPath.size(); i+=3){
                glm::vec2 newPoint = glm::vec2(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ (yFlip * ofToFloat(splitGlyphPath[i+2])* scale));
//                if(lastChar == " ") cout<<"SPACE";
                if((i == 0 && c == 0) || (i == 0 && lastChar == " ")){
                    //first letter and first point in each is a moveto
                    charPath.moveTo(newPoint.x,newPoint.y);
                } else if(singlePath == true && i == 0){
                    //based on distance we decide if we use move or line between letters
                    //                    glm::vec2 newPoint = glm::vec2(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ (yFlip * ofToFloat(splitGlyphPath[i+2])* scale));
                    if(distance(lastPoint,newPoint) > (charWidth*scale*0.5)){
                        charPath.moveTo(newPoint.x,newPoint.y);
                    } else {
                        charPath.lineTo(newPoint.x,newPoint.y);
                    }
                    //                    charPath.lineTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
                }else{
                    //follow font move/line settings within a letter
                    if(splitGlyphPath[i] == "M"){
                        charPath.moveTo(newPoint.x,newPoint.y);
                    }else if(splitGlyphPath[i] == "L"){
                        charPath.lineTo(newPoint.x,newPoint.y);
                    }
                }
                
                lastPoint = newPoint;
                
            }//end for(int i=0; i<splitGlyphPath.size(); i+=3)
            x += (charWidth*scale);
            //            cout<<endl;
            lastChar = nextChar;
        }//end  for (int c = 0; c < lines[l].size(); c++)
        
        y += getLineHeight(scale);
        
    }//end for (int l = 0; l < lines.size(); l++)
    

    /*
     for (int i = 0; i < stringValue.size(); i++)
     {
     
     //        string nextChar = stringValue.substr (i,1); //stringValue.at(i);
     string nextChar = ofUTF8Substring(stringValue, i, 1);
     //        ofLog()<<"nextChar.length "<<nextChar.length();
     if(nextChar.length() == 0) continue; //skip empty char which somehow ö has
     
     //        string nextChar = ofToString(stringValue.at(i));
     //        ofLog()<<"nextChar "<<nextChar;
     bool firstLetterInWord = false;
     if(i == 0){
     firstLetterInWord = true;
     }else{
     string prevChar = ofUTF8Substring(stringValue, i-1, 1); //ofToString(stringValue.at(i-1));
     if(prevChar == delimiter || prevChar == " ") firstLetterInWord = true;
     } 
     if(nextChar == delimiter){
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
     glm::vec2 newPoint = glm::vec2(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ (yFlip * ofToFloat(splitGlyphPath[i+2])* scale));
     
     if(firstLetterInWord == true){
     firstLetterInWord = false;
     charPath.moveTo(newPoint.x,newPoint.y);
     } else if(singlePath == true && i == 0 && firstLetterInWord == false){
     //                    glm::vec2 newPoint = glm::vec2(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ (yFlip * ofToFloat(splitGlyphPath[i+2])* scale));
     if(distance(lastPoint,newPoint) > (charWidth*scale*0.5)){
     charPath.moveTo(newPoint.x,newPoint.y);
     } else {
     charPath.lineTo(newPoint.x,newPoint.y);
     }
     //                    charPath.lineTo(x+ofToFloat(splitGlyphPath[i+1])* scale, y+ yFlip * ofToFloat(splitGlyphPath[i+2])* scale);
     }else{
     if(splitGlyphPath[i] == "M"){
     charPath.moveTo(newPoint.x,newPoint.y);
     }else if(splitGlyphPath[i] == "L"){
     charPath.lineTo(newPoint.x,newPoint.y);
     }
     }
     
     lastPoint = newPoint;
     }
     }
     x += (charWidth*scale);
     }
     */
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

