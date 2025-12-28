#ifndef FILTERS_H
#define FILTERS_H

class Filter {
public:
    Filter();
    virtual ~Filter();
    
    virtual float apply(float input) = 0;
    virtual void reset();
    
protected:
    float lastOutput;
};

class MovingAverageFilter : public Filter {
public:
    MovingAverageFilter(int windowSize);
    virtual float apply(float input) override;
    virtual void reset() override;
    
private:
    int windowSize;
    float* buffer;
    int index;
    int count;
    float sum;
};

class MedianFilter : public Filter {
public:
    MedianFilter(int windowSize);
    virtual float apply(float input) override;
    virtual void reset() override;
    
private:
    int windowSize;
    float* buffer;
    int index;
    int count;
};

class KalmanFilter : public Filter {
public:
    KalmanFilter(float processNoise, float measurementNoise);
    virtual float apply(float input) override;
    virtual void reset() override;
    
private:
    float processNoise;
    float measurementNoise;
    float estimate;
    float error;
    float kalmanGain;
};

#endif // FILTERS_H