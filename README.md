# Calyp
## Enhanced YUV video player based on Qt focusing on image/video analysis

## Build Status

Linux | Windows
------------ | -------------
[Build Action](https://github.com/pixlra/calyp/actions/workflows/build.yml/badge.svg?branch=master) |
[![Linux Status](https://travis-ci.org/pixlra/calyp.svg?branch=master)](https://travis-ci.org/pixlra/calyp.svg?branch=master) | [![Windows status](https://ci.appveyor.com/api/projects/status/ea2ktvtg7uqwrika/branch/master?svg=true)](https://ci.appveyor.com/project/jfmcarreira/calyp/branch/master)


## Features
- Support for libavformat and libavcodec;
- Support for wide range of raw video formats;
- Support up to 16 bits per pixel
- Support for OpenCV image/video processing library
- Advanced frame zoom with easy to use pan function
- Synchronized zoom across several videos (perfect for comparison operations)
- Useful information in status bar, *e.g.*, pixel information
    Useful sidebars with frame information, *e.g.*, frame histogram
- Constant growing libs for stream and frame processing (CalypStream and CalypFrame)
- Advanced API for frame/video processing algorithms
- Frame level quality measurement API based on the CalypFrame class
- Powerful command-line tool for quality and frame processing algorithms (uses the referred APIs)

## Modules
- CalypLib: Low level library
- CalypModules: Abstract interface for frame processing modules
- Calyp: High level graphical interface
- CalypTools: High level command line interface

##  Supported Quality Metrics
- Peak Signal-to-Noise Ratio (PSNR)
- Structural SIMilarity (SSIM)
- Mean Square Error (MSE)
- Weighted Spherical PSNR (WS-PSNR)

## Frame Processing Modules
- Component filtering
- Frame difference, crop, shift, binarization and rotation
- Sub-sampling operations
- Measuring modules
- Modules based on opencv:
    - Disparity estimation
    - Motion estimation

## Supported Formats
- Supports for the following pixel formats:
    * YUV: 444, 420, 422
    * RGB
    * Gray
- Supported formats:
    * Raw video:
        * YUV: 444, 420, 422
        * RGB
        * Gray
    * Joint Photographic Experts Group (jpeg)
    * Portable Map (ppm, pgm)
- Partially supported formats:
    * Portable Network Graphics (png)
    * Windows Bitmap (bmp)
    * Tagged Image File Format (tiff)

## Development Team
- [**Joao Carreira**](https://github.com/jfmcarreira) (Founder, Main Developer)
- [**Lui­s Lucas**   ](https://github.com/lfrlucas)    (Founder, Developer)
- [**Joao Santos**  ](https://github.com/jpataias)    (Contributor)
- [**JNSFilipe**    ](https://github.com/JNSFilipe)   (Contributor)


### Binaries and Build
* Packages are available for [Debian](https://launchpad.net/~pixlra/+archive/ubuntu/ppa)
* **Daily builds** available for [Windows](https://github.com/pixlra/calyp-releases/blob/master/installers/win/calyp-latest.exe)
* **Dependencies**
  1. an ordinary C++ development environment (g++, make, C++ libraries and headers, ...)
  2. cmake
  3. a Qt development environment: QtCore, QtGui, QtWidgets, QtPrintSupport and QtConcurrent (version 5 or higher)
  4. Optional: QtDBus, OpenCV, FFmpeg
* **List of required commands**
```
mkdir build
cd build
cmake -DUSE_FFMPEG=ON -DUSE_OPENCV=ON ..  (for FFmpeg and OpenCV support)
make
make install
```
