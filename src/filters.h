#ifndef FILTERS_H
#define FILTERS_H

#ifdef THREAD_SAFE_FILTERS
#include <mutex>
#endif

template<typename T = float>
class Filter {
public:
    Filter() : lastOutput(T(0)) {}
    virtual ~Filter() {}
    
    virtual T apply(T input) = 0;
    virtual void reset() {
        lastOutput = T(0);
    }
    
protected:
    T lastOutput;
#ifdef THREAD_SAFE_FILTERS
    std::mutex mtx;
#endif
};

template<typename T = float>
class MovingAverageFilter : public Filter<T> {
public:
    MovingAverageFilter(int windowSize);
    virtual T apply(T input) override;
    virtual void reset() override;
    
private:
    int windowSize;
    T* buffer;
    int index;
    int count;
    T sum;
};

template<typename T = float>
class MedianFilter : public Filter<T> {
public:
    MedianFilter(int windowSize);
    virtual T apply(T input) override;
    virtual void reset() override;
    
private:
    int windowSize;
    T* buffer;
    int index;
    int count;
    void sortBuffer(T* arr, int size);
};

template<typename T = float>
class EMAFilter : public Filter<T> {
public:
    EMAFilter(T alpha);
    virtual T apply(T input) override;
    virtual void reset() override;
    
private:
    T alpha;
    bool initialized;
};

template<typename T = float>
class KalmanFilter : public Filter<T> {
public:
    KalmanFilter(T processNoise, T measurementNoise);
    virtual T apply(T input) override;
    virtual void reset() override;
    
private:
    T processNoise;
    T measurementNoise;
    T estimate;
    T error;
    T kalmanGain;
};

#endif // FILTERS_H