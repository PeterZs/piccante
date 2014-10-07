/*

PICCANTE
The hottest HDR imaging library!
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

PICCANTE is free software; you can redistribute it and/or modify
under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 3.0 of
the License, or (at your option) any later version.

PICCANTE is distributed in the hope that it will be useful, but
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License
( http://www.gnu.org/licenses/lgpl-3.0.html ) for more details.

*/

#ifndef PIC_UTIL_GL_STROKE_HPP
#define PIC_UTIL_GL_STROKE_HPP

#include "gl/image_raw.hpp"
#include "util/gl/quad.hpp"
#include "externals/glw/program.hpp"

namespace pic {

//TODO: removing immediate mode

/**
 * @brief The StrokeGL class
 */
class StrokeGL
{
protected:
    int					width, height, brushSize;
    ImageRAWGL			*shape;
    float				size, rSize;
    float				color[4];
    float				tmpColor[3];

    QuadGL              *quad;

    std::vector<float>	positions;

    glw::program		annotationProgram;
    glw::program        brushProgram;

public:
    float				annotation;

    /**
     * @brief StrokeGL
     * @param width
     * @param height
     * @param brushSize
     * @param color
     */
    StrokeGL(int width, int height, int brushSize, float *color);

    ~StrokeGL();

    /**
     * @brief SetupShaders
     */
    void SetupShaders();

    /**
     * @brief Resample
     */
    void Resample();

    /**
     * @brief Reset
     */
    void Reset();

    /**
     * @brief Insert2DPoint
     * @param x
     * @param y
     */
    void Insert2DPoint(int x, int y);

    /**
     * @brief RenderGL
     */
    void RenderGL();

    /**
     * @brief RenderAnnotationGL
     */
    void RenderAnnotationGL();

    /**
     * @brief RenderBrushGL
     * @param x
     * @param y
     */
    void RenderBrushGL(int x, int y);

    /**
     * @brief Size
     * @return
     */
    unsigned int Size()
    {
        return positions.size();
    }

    /**
     * @brief bindCol
     * @param val
     */
    void bindCol(float val)
    {
        memcpy(tmpColor, color, 3 * sizeof(float));
        color[0] = color[1] = color[2] = val;
    }

    /**
     * @brief unBindCol
     */
    void unBindCol()
    {
        memcpy(color, tmpColor, 3 * sizeof(float));
    }

    /**
     * @brief Straightner
     */
    void Straightner()
    {
        if(positions.size() > 3) {
            float x0 = positions[0];
            float y0 = positions[1];

            int n = positions.size() - 2;
            float x1 = positions[n];
            float y1 = positions[n + 1];

            positions.clear();
            positions.push_back(x0);
            positions.push_back(y0);
            positions.push_back(x1);
            positions.push_back(y1);
        }
    }
};

StrokeGL::StrokeGL(int width, int height, int brushSize = 128,
                 float *color = NULL)
{
    annotation = 1.0f;


    this->width = width;
    this->height = height;

    if(brushSize > 1) {
        this->brushSize = brushSize;
    } else {
        this->brushSize = 2;
    }

    int halfBrushSize = this->brushSize >> 1;

    float halfBrushSizeXf = float(halfBrushSize) / float(width);
    float halfBrushSizeYf = float(halfBrushSize) / float(height);
    printf("%f %f\n", halfBrushSizeXf, halfBrushSizeYf);
    this->quad = new QuadGL(true, halfBrushSizeXf, halfBrushSizeYf);

    size = 4.0f;
    rSize = size / float(max(width, height));

    shape = new ImageRAWGL(1, this->brushSize, this->brushSize, 3, IMG_CPU, GL_TEXTURE_2D);
//	shape->EvaluateGaussian(true);
    shape->EvaluateSolid();
    shape->generateTextureGL(false, GL_TEXTURE_2D);

    if(color != NULL) {
        for(int i = 0; i < 3; i++) {
            this->color[i] = color[i];
        }
    } else {
        this->color[0] = this->color[1] = this->color[2] = 1.0f;
    }

    SetupShaders();
}

StrokeGL::~StrokeGL()
{
    if(shape != NULL) {
        delete shape;
    }

    shape = NULL;
}

void StrokeGL::SetupShaders()
{
    //common vertex program
    std::string vertex_source = GLW_STRINGFY
                                (
    in vec2 a_position;
    in vec2 a_tex_coord;
    uniform vec2 shift_position;
    out vec2 v_tex_coord;

    void main(void) {
        v_tex_coord  = a_tex_coord;
        gl_Position = vec4(a_position + shift_position, 0.0, 1.0);
    }
                                );

    //rendering
    std::string fragment_source = GLW_STRINGFY
                                  (
    uniform sampler2D   u_tex;
    uniform vec4        current_color;
    in vec2             v_tex_coord;
    out vec4            f_color;

    void main(void) {
        float shape = texture2D(u_tex, v_tex_coord).x;
        f_color = vec4(current_color * shape);
    }
                                  );

    brushProgram.setup(glw::version("330"), vertex_source, fragment_source);

    printf("[StrokeGL log]\n%s\n", brushProgram.log().c_str());
    
    glw::bind_program(brushProgram);
    brushProgram.attribute_source("a_position", 0);
    brushProgram.attribute_source("a_tex_coord", 1);
    brushProgram.fragment_target("f_color", 0);
    brushProgram.relink();

    brushProgram.uniform("u_tex", 0);
    brushProgram.uniform("shift_position", 0.0f, 0.0f);
    brushProgram.uniform("current_color", 1.0f, 1.0f, 1.0f, 1.0f);
    glw::bind_program(0);

    //Annotation
    std::string fragment_source_annotation = GLW_STRINGFY
                                  (
    uniform sampler2D   u_tex;
    uniform float       annotation;
    in vec2             v_tex_coord;
    out vec4            f_color;

    void main(void) {
        float shape = texture2D(u_tex, v_tex_coord).x;
        float shapeVal = shape * annotation;
        f_color = vec4(shapeVal, shapeVal, shapeVal, shape);
    }
                                  );

    annotationProgram.setup(glw::version("330"), vertex_source, fragment_source_annotation);

    printf("[StrokeGL log]\n%s\n", annotationProgram.log().c_str());


    glw::bind_program(annotationProgram);
    annotationProgram.attribute_source("a_position", 0);
    annotationProgram.attribute_source("a_tex_coord", 1);
    annotationProgram.fragment_target("f_color", 0);
    annotationProgram.relink();

    annotationProgram.uniform("u_tex", 0);
    annotationProgram.uniform("shift_position", 0.0f, 0.0f);
    annotationProgram.uniform("annotation", annotation);
    glw::bind_program(0);
}

void StrokeGL::Resample()
{
    if(positions.size() <= 0) {
        return;
    }

    bool *sampleGrid = MaskSetValue(NULL, height * width, false);

    //Calculate length of the path
    float len = 0.0f;
    float tmpLen;
    const int n = size_t(positions.size()) - 2;
    float x, y;
    std::vector<float> lengths;

    for(int i = 0; i < n; i += 2) {
        x = positions[i + 2] - positions[i];
        y = positions[i + 3] - positions[i + 1];
        tmpLen = sqrtf(x * x + y * y);
        lengths.push_back(tmpLen);
        len += tmpLen;
    }

    //Resampling
    std::vector<float> resampledPos;
    resampledPos.push_back(positions[0]);
    resampledPos.push_back(positions[1]);

    float workLen = 0.0f;
    int nSamples = int(len / (rSize * 0.25f));
    float deltaL = len / float(nSamples);

#ifdef PIC_DEBUG
    printf("Len: %f Samples: %d DeltaL: %f\n", len, nSamples, deltaL);
#endif

    for(int i = 1; i < nSamples; i++) {
        workLen += deltaL;
        unsigned int j;
        bool test = false;
        float tmpWork = 0.0f;

        for(j = 0; j < lengths.size(); j++) {
            if(workLen >= tmpWork && workLen < (tmpWork + lengths[j])) {
                test = true;
                break;
            }

            tmpWork += lengths[j];
        }

        if(!test) {
            break;
        }

        int ind = j << 1;

        float x0 = positions[ind];
        float y0 = positions[ind + 1];

        float x1 = positions[ind + 2];
        float y1 = positions[ind + 3];

        float shift = (deltaL - (tmpWork - workLen)) / lengths[j];

        x =  shift * (x1 - x0) + x0;
        y =  shift * (y1 - y0) + y0;

        int tmpX = CLAMP(int((x / 2.0f + 0.5f) * width), width);
        int tmpY = CLAMP(int((y / 2.0f + 0.5f) * height), height);
        int indSG = tmpY * width + tmpX;
        bool tmpSampleGrid = sampleGrid[indSG];

//		float *val = ((*sampleGrid)(x/2.0f+0.5,y/2.0f+0.5f));

        if(tmpSampleGrid == false) {
            resampledPos.push_back(x);
            resampledPos.push_back(y);
            sampleGrid[indSG] = true;
        }

        //printf("%f %f %d %f\n",x,y,j,workLen);
        workLen += deltaL;

        if(workLen > len) {
            break;
        }
    }

    positions.clear();
    positions.insert(positions.begin(), resampledPos.begin(), resampledPos.end());

    if(sampleGrid != NULL) {
        delete[] sampleGrid;
    }
}

void StrokeGL::Insert2DPoint(int x, int y)
{
    /*
    	float xf = -(x/float(width) -0.5f)*2.0f;
    	float yf =  (y/float(height)-0.5f)*2.0f;
    */
    float xf = (x / float(width) - 0.5f) * 2.0f;
    float yf = (y / float(height) - 0.5f) * 2.0f;

    positions.push_back(xf);
    positions.push_back(yf);
}

void StrokeGL::Reset()
{
    positions.clear();
}

void StrokeGL::RenderBrushGL(int x, int y)
{
    float xf = (x / float(width)  - 0.5f) * 2.0f;
    float yf = (y / float(height) - 0.5f) * 2.0f;

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    shape->bindTexture();
      
    glw::bind_program(brushProgram);
        
    if(annotation < 0.0f) {
        brushProgram.uniform("current_color", 1.0f - color[0], 1.0f - color[1], 1.0f - color[2], 1.0f);
    } else {
        brushProgram.uniform("current_color", color[0], color[1], color[2], 0.5f);
    }
    
    brushProgram.uniform("shift_position", xf, yf);

    quad->Render();

    glw::bind_program(0);

    shape->unBindTexture();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void StrokeGL::RenderGL()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glEnable(GL_TEXTURE_2D);

    glActiveTexture(GL_TEXTURE0);
    shape->bindTexture();

    glBindTexture(GL_TEXTURE_2D, shape->getTexture());

    glw::bind_program(brushProgram);

    brushProgram.uniform("current_color", color[0], color[1], color[2], 1.0f);
    const int n = size_t(positions.size());

    for(int i = 0; i < n; i += 2) {
        brushProgram.uniform("shift_position", positions[i], positions[i + 1]);
        quad->Render();
    }
    glEnd();

    glw::bind_program(0);

    shape->unBindTexture();

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void StrokeGL::RenderAnnotationGL()
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    glw::bind_program(annotationProgram);

#ifdef PIC_DEBUG
    printf("Annotation value: %f\n", annotation);
#endif
    annotationProgram.uniform("annotation",  annotation);

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    shape->bindTexture();

    const int n = size_t(positions.size());

    for(int i = 0; i < n; i += 2) {
        annotationProgram.uniform("shift_position", positions[i], positions[i + 1]);
        quad->Render();
    }

    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    glw::bind_program(0);
    glDisable(GL_BLEND);
}

/*
    if(bPointSprite) {
        glEnable(GL_POINT_SPRITE);
        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
        glPointSize(GLfloat(brushSize));
        glBegin(GL_POINTS);
        glColor3fv(color);
        const int n = size_t(positions.size());

        for(int i = 0; i < n; i += 2) {
            glVertex2f(positions[i], positions[i + 1]);
        }

        glEnd();

        //glDisable(GL_POINT_SMOOTH);
        glDisable(GL_POINT_SPRITE);
*/


} // end namespace pic

#endif /* PIC_UTIL_GL_STROKE_HPP */