#ifndef CV_UTIL_H_

#include "qsgModel.h"
#include <QImage>
#include "opencv2/opencv.hpp"

QImage cvMat2QImage(const cv::Mat& mat);
cv::Mat QImage2cvMat(const QImage& image);
std::vector<std::vector<rea::pointList>> extractContour(const QPoint& aTopLeft, const QImage& aImage, int aEpsilon, int aThreshold = 50);

#endif
