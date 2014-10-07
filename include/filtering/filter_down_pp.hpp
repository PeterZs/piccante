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

#ifndef PIC_FILTERING_FILTER_DOWN_PP_HPP
#define PIC_FILTERING_FILTER_DOWN_PP_HPP

#include "filtering/filter.hpp"

namespace pic {

/**
 * @brief The FilterDownPP class
 */
class FilterDownPP: public Filter
{
protected:
    /**
     * @brief ProcessBBox
     * @param dst
     * @param src
     * @param box
     */
    void ProcessBBox(ImageRAW *dst, ImageRAWVec src, BBox *box)
    {
        int channels = src[0]->channels;

        for(int i2 = box->y0; i2 < box->y1; i2++) {
            int i = i2 << 1;

            for(int j2 = box->x0; j2 < box->x1; j2++) {
                int j = j2 << 1;

                float *tmp[4];
                tmp[0] = (*src[0])(j    , i);
                tmp[1] = (*src[0])(j + 1, i);
                tmp[2] = (*src[0])(j    , i + 1);
                tmp[3] = (*src[0])(j + 1, i + 1);

                int counter = 0;
                float *tmp_ret = (*dst)(j2, i2);

                for(int l = 0; l < channels; l++) {
                    tmp_ret[l] = 0.0f;
                }

                for(unsigned k = 0; k < 4; k++) {
                    if(distance(tmp[k], value, channels) > threshold) {
                        counter++;

                        for(int l = 0; l < channels; l++) {
                            tmp_ret[l] += tmp[k][l];
                        }
                    }
                }

                if(counter > 0) {
                    float counter_f = float(counter);
                    for(int l = 0; l < channels; l++) {
                        tmp_ret[l] /= counter_f;
                    }
                } else {
                    for(int l = 0; l < channels; l++) {
                        tmp_ret[l] = value[l];
                    }
                }
            }
        }
    }

    /**
     * @brief SetupAux
     * @param imgIn
     * @param imgOut
     * @return
     */
    ImageRAW *SetupAux(ImageRAWVec imgIn, ImageRAW *imgOut)
    {
        if(imgOut == NULL) {
            imgOut = new ImageRAW(1, imgIn[0]->width >> 1, imgIn[0]->height >> 1, imgIn[0]->channels);
        } else {
            if(((imgIn[0]->width >> 1)  != imgOut->width )  ||
               ((imgIn[0]->height >> 1) != imgOut->height) ) {
                imgOut = new ImageRAW(1, imgIn[0]->width >> 1, imgIn[0]->height >> 1, imgIn[0]->channels);
            }
        }

        return imgOut;
    }

    float *value, threshold;

public:

    /**
     * @brief FilterDownPP
     * @param value
     * @param threshold
     */
    FilterDownPP(float *value, float threshold)
    {
        Update(value, threshold);
    }

    ~FilterDownPP()
    {
    }

    /**
     * @brief distance
     * @param a
     * @param b
     * @param channels
     * @return
     */
    static inline float distance(float *a, float *b, int channels)
    {
        float dist = 0.0f;
        for(int i = 0; i < channels; i++) {
            float tmp = a[i] - b[i];
            dist += tmp * tmp;
        }

        return dist;
    }

    /**
     * @brief Update
     * @param value
     * @param threshold
     */
    void Update(float *value, float threshold)
    {
        if(value == NULL) {
            this->value = new float[3];

            for(int i = 0; i < 3; i++) {
                this->value[i] = 0.0f;
            }
        } else {
            this->value = value;
        }

        if(threshold > 0.0f) {
            this->threshold = threshold;
        } else {
            this->threshold = 1e-4f;
        }
    }

    /**
     * @brief OutputSize
     * @param imgIn
     * @param width
     * @param height
     * @param channels
     * @param frames
     */
    void OutputSize(ImageRAW *imgIn, int &width, int &height, int &channels, int &frames)
    {
        width       = imgIn->width  >> 1;
        height      = imgIn->height >> 1;
        channels    = imgIn->channels;
        frames      = imgIn->frames;
    }

    /**
     * @brief Execute
     * @param imgIn
     * @param imgOut
     * @param type
     * @return
     */
    static ImageRAW *Execute(ImageRAW *imgIn, ImageRAW *imgOut)
    {
        FilterDownPP flt(NULL, 1e-3f);
        return flt.ProcessP(Single(imgIn), imgOut);
    }

    /**
     * @brief Execute
     * @param fileInput
     * @param fileOutput
     * @return
     */
    static ImageRAW *Execute(std::string fileInput, std::string fileOutput)
    {
        ImageRAW imgIn(fileInput);
        ImageRAW *out = FilterDownPP::Execute(&imgIn, NULL);
        out->Write(fileOutput);
        return out;
    }
};

} // end namespace pic

#endif /* PIC_FILTERING_FILTER_DOWN_PP_HPP */

