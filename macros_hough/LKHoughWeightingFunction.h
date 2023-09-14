#ifndef LKHOUGHWEIGHTINGFUNCTION_HH
#define LKHOUGHWEIGHTINGFUNCTION_HH

#include "LKImagePoint.h"
#include "LKHoughPointRT.h"

#include "LKImagePoint.cpp"
#include "LKHoughPointRT.cpp"

class LKHoughWeightingFunction
{
    public:
        LKHoughWeightingFunction() {}
        ~LKHoughWeightingFunction() {}

        virtual double EvalFromPoints(LKImagePoint* imagePoint, LKHoughPointRT* houghPoint) {
            auto distance = houghPoint -> DistanceToImagePoint(0, imagePoint);
            auto error = imagePoint -> GetError();
            return EvalFromDistance(distance,error,imagePoint->fWeight);
        }

        virtual double EvalFromDistance(double distance, double error, double pointWeight) {
            return 1;
        }
};

class LKHoughWFConst : public LKHoughWeightingFunction
{
    public:
        LKHoughWFConst() {}
        ~LKHoughWFConst() {}

        double EvalFromPoints(LKImagePoint* imagePoint, LKHoughPointRT* houghPoint) { return 1; }
        double EvalFromDistance(double distance, double error, double pointWeight) { return 1; }
};

class LKHoughWFLinear : public LKHoughWeightingFunction
{
    public:
        LKHoughWFLinear() {}
        ~LKHoughWFLinear() {}

        double EvalFromDistance(double distance, double error, double pointWeight) {
            double weight = pointWeight * (1 - distance/error/2);
            if (weight<0)
                return 0;
            return weight;
        }
};

class LKHoughWFInverse : public LKHoughWeightingFunction
{
    public:
        LKHoughWFInverse() {}
        ~LKHoughWFInverse() {}

        double EvalFromDistance(double distance, double error, double pointWeight) {
            double weight = pointWeight * (error)/(distance+error);
            return weight;
        }
};
#endif