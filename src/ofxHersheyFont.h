/*
 *  ofxHersheyFont.h
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

#include "ofMain.h"

class ofxHersheyFont {
public:
    ofxHersheyFont();
    
    void draw(string stringValue, float x, float y, float scale){
        draw(stringValue, x, y, scale, false, 0);
    }
    void draw(string stringValue, float x, float y, float scale, bool centered){
        draw(stringValue, x, y, scale, centered, 0);
    }
    void draw(string stringValue, float x, float y, float scale, bool centered, float angle);
    void drawSpecial(string stringValue, float xPos, float yPos, float scale, bool centered, float angle);
    
    //    void drawResampled(string stringValue, float x, float y, float scale, bool centered, float angle, float _resampleSpacing);
    ofColor color;
    void setColor(ofColor c);
    //    glm::vec2 getDimensions(string stringValue, float scale);
    ofRectangle getBoundingBox(string stringValue, float scale = 1, glm::vec2 pos = glm::vec2(0,0));
    
    float getHeight(string stringValue,float scale);
    float getWidth(string stringValue,float scale);
    
    float getLineHeight(float scale);
    float lineHeight;
    
    string tallestChar;
    float tallestChar_height;
    void getTallestChar(string & _char, float & _height);
    
    ofPath getPath(string stringValue, float scale, int yFlip = -1){
        return getPath(stringValue, 0, 0, scale, yFlip);
    };
    //    ofPath getPath(string stringValue, float xPos, float yPos, float scale, int yFlip = -1){
    //        return getPath(stringValue, xPos, yPos, scale, -1, yFlip);
    //    }
    ofPath getPath(string stringValue, float x, float y, float scale, int yFlip = -1);
    ofPath getPathSpecial(string stringValue, float x, float y, float scale, int yFlip = -1);
    
    string getPath_asPythonString(string stringValue, string prefix, float xPos, float yPos, float scale, int yFlip = -1);
 
   
    
    ofXml svgFontFile;
    void loadSVGFont(filesystem::path _fontPath);

    filesystem::path fontPath;
    string fontName;
    string currentPhrase;
    float currentScale;
    ofRectangle boundingBox;
    ofParameter<string> delimiter; // = "\n";
    
    ofParameter<float> resampleSpacing = 0;
    ofParameter<int> smoothingSize = 0;
    ofParameter<float> smoothingShape = 0;
    
private:
    void drawChar(ofXml xmlElement);
};


