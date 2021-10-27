/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2021  by Joao Carreira   (jfmcarreira@gmail.com)
 *                                Luis Lucas      (luisfrlucas@gmail.com)
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file     PixelFormats.cpp
 * \brief    Handling the pixel formats definition
 */

#include "PixelFormats.h"

#include "CalypFrame.h"
#include "config.h"
#ifdef USE_FFMPEG
#include "StreamHandlerLibav.h"
#endif

/*
 **************************************************************
 * Handle different pel formats
 * interface to generalise into one struct the pixel formats
 **************************************************************
 */

#ifdef USE_FFMPEG
#define ADD_FFMPEG_PEL_FMT( fmt ) fmt
#else
#define ADD_FFMPEG_PEL_FMT( fmt ) 0
#endif

const std::map<ClpPixelFormats, CalypPixelFormatDescriptor> g_CalypPixFmtDescriptorsMap = {
    {
        ClpPixelFormats::YUV420p,
        {
            "YUV420p"sv,
            CLP_COLOR_YUV,
            3,
            3,
            1,
            1,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_YUV420P ),
            {
                { 0, 0, 1 }, /* Y */
                { 1, 0, 1 }, /* U */
                { 2, 0, 1 }, /* V */
            },
        },
    },
    {
        ClpPixelFormats::YUV422p,
        {
            "YUV422p"sv,
            CLP_COLOR_YUV,
            3,
            3,
            1,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_YUV422P ),
            {
                { 0, 0, 1 }, /* Y */
                { 1, 0, 1 }, /* U */
                { 2, 0, 1 }, /* V */
            },
        },
    },
    {
        ClpPixelFormats::YUV444p,
        {
            "YUV444p"sv,
            CLP_COLOR_YUV,
            3,
            3,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_YUV444P ),
            {
                { 0, 0, 1 }, /* Y */
                { 1, 0, 1 }, /* U */
                { 2, 0, 1 }, /* V */
            },
        },
    },
    {
        ClpPixelFormats::YUYV422,
        {
            "YUYV422"sv,
            CLP_COLOR_YUV,
            3,
            1,
            1,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_YUYV422 ),
            {
                { 0, 1, 1 }, /* Y */
                { 0, 3, 2 }, /* U */
                { 0, 3, 4 }, /* V */
            },
        },
    },
    {
        ClpPixelFormats::Gray,
        {
            "GRAY"sv,
            CLP_COLOR_GRAY,
            1,
            1,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_GRAY8 ),
            { { 0, 0, 1 } }, /* Y */
        },
    },
    {
        ClpPixelFormats::RGB24p,
        {
            "RGBp"sv,
            CLP_COLOR_RGB,
            3,
            3,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_NONE ),
            {
                { 0, 0, 1 }, /* R */
                { 1, 0, 1 }, /* G */
                { 2, 0, 1 }, /* B */
            },
        },
    },
    {
        ClpPixelFormats::RGB24,
        {
            "RGB"sv,
            CLP_COLOR_RGB,
            3,
            1,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_RGB24 ),
            {
                { 0, 2, 1 }, /* R */
                { 0, 2, 2 }, /* G */
                { 0, 2, 3 }, /* B */
            },
        },
    },
    {
        ClpPixelFormats::BGR24,
        {
            "BGR"sv,
            CLP_COLOR_RGB,
            3,
            1,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_BGR24 ),
            {
                { 0, 2, 3 }, /* R */
                { 0, 2, 2 }, /* G */
                { 0, 2, 1 }, /* B */
            },
        },
    },
    {
        ClpPixelFormats::RGBA32,
        {
            "RGBA"sv,
            CLP_COLOR_RGBA,
            4,
            1,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_RGBA ),
            {
                { 0, 3, 1 }, /* R */
                { 0, 3, 2 }, /* G */
                { 0, 3, 3 }, /* B */
                { 0, 3, 4 }, /* A */
            },
        },
    },
    {
        ClpPixelFormats::BGRA32,
        {
            "BGRA"sv,
            CLP_COLOR_RGBA,
            4,
            1,
            0,
            0,
            ADD_FFMPEG_PEL_FMT( AV_PIX_FMT_BGRA ),
            {
                { 0, 3, 3 }, /* R */
                { 0, 3, 2 }, /* G */
                { 0, 3, 1 }, /* B */
                { 0, 3, 4 }, /* A */
            },
        },
    },
};
