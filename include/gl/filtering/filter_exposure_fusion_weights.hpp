/*

PICCANTE
The hottest HDR imaging library!
http://vcg.isti.cnr.it/piccante

Copyright (C) 2014
Visual Computing Laboratory - ISTI CNR
http://vcg.isti.cnr.it
First author: Francesco Banterle

This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at http://mozilla.org/MPL/2.0/.

*/

#ifndef PIC_GL_FILTERING_FILTER_EXPOSURE_FUSION_WEIGHTS_HPP
#define PIC_GL_FILTERING_FILTER_EXPOSURE_FUSION_WEIGHTS_HPP

#include "gl/filtering/filter.hpp"

namespace pic {

/**
 * @brief The FilterGLExposureFusionWeights class
 */
class FilterGLExposureFusionWeights: public FilterGL
{
protected:
    /**
     * @brief InitShaders
     */
    void InitShaders();

    /**
     * @brief FragmentShader
     */
    void FragmentShader();

    float sigma2, mu;
    float wC, wE, wS;

public:

    /**
     * @brief FilterGLExposureFusionWeights
     * @param wC
     * @param wE
     * @param wS
     */
    FilterGLExposureFusionWeights(float wC = 1.0f, float wE = 1.0f, float wS = 1.0f) : FilterGL()
    {
        float sigma = 0.2f;

        mu = 0.5f;
        sigma2 = 2.0f * sigma * sigma;

        this->wC = wC > 0.0f ? wC : 1.0f;
        this->wE = wE > 0.0f ? wE : 1.0f;
        this->wS = wS > 0.0f ? wS : 1.0f;

        //protected values are assigned/computed
        FragmentShader();
        InitShaders();
    }
};

void FilterGLExposureFusionWeights::FragmentShader()
{
    fragment_source = GLW_STRINGFY
                      (
                          uniform sampler2D u_tex; \n
                          uniform sampler2D u_tex_lum; \n
                          uniform float wC;   \n
                          uniform float wE;   \n
                          uniform float wS;   \n
                          out vec4      f_color;	\n

    void main(void) {
        \n
        ivec2 coords = ivec2(gl_FragCoord.xy);\n

        float L = texelFetch(u_tex_lum, coords, 0).x; \n

        //wC
        float pCon = -4.0 * L;
        pCon += texelFetch(u_tex_lum, coords + ivec2(1, 0), 0).x;\n
        pCon += texelFetch(u_tex_lum, coords - ivec2(1, 0), 0).x;\n
        pCon += texelFetch(u_tex_lum, coords + ivec2(0, 1), 0).x;\n
        pCon += texelFetch(u_tex_lum, coords - ivec2(0, 1), 0).x;\n
        f_color = vec4(pow(vec3(pCon), vec3(wC)), 1.0);
    }\n
                      );
}

void FilterGLExposureFusionWeights::InitShaders()
{
    FragmentShader();

    std::string prefix;

    filteringProgram.setup(glw::version("330"), vertex_source, fragment_source);

#ifdef PIC_DEBUG
    printf("[FilterGLExposureFusionWeights log]\n%s\n", filteringProgram.log().c_str());
#endif

    glw::bind_program(filteringProgram);
    filteringProgram.attribute_source("a_position", 0);
    filteringProgram.fragment_target("f_color",    0);
    filteringProgram.relink();
    filteringProgram.uniform("u_tex", 0);
    filteringProgram.uniform("u_tex_lum", 1);
    filteringProgram.uniform("wC", wC);
    filteringProgram.uniform("wE", wE);
    filteringProgram.uniform("wS", wS);
    glw::bind_program(0);
}

} // end namespace pic

#endif /* PIC_GL_FILTERING_FILTER_EXPOSURE_FUSION_WEIGHTS_HPP */

