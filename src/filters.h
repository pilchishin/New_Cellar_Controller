#ifndef FILTERS_H
#define FILTERS_H

// #ifdef THREAD_SAFE_FILTERS
// #include <mutex>
// #endif

    /**
     * @brief Канал обработки данных с каскадной фильтрацией
     */
class FilteredChannel {
public:
    /**
     * @brief Конструктор канала фильтрации.
     * Инициализирует медианный фильтр и EMA-фильтр с заданными параметрами.
     *
     * @param medianWindow Размер окна медианного фильтра. Должен быть нечётным и > 0.
     * @param emaAlpha Коэффициент сглаживания EMA-фильтра. Диапазон: (0.0 … 1.0).
     */
    FilteredChannel(uint8_t medianWindow, float emaAlpha);
    
    /**
     * @brief Сброс состояния фильтров.
     *
     * Очищает внутренние буферы медианного фильтра,
     * сбрасывает состояние EMA и последнее сохранённое значение.
     */
    void reset();

        /**
     * @brief Обновление канала новым сырым значением.
     *
     * Передаёт входное значение в медианный фильтр, затем результат обрабатывается EMA-фильтром.
     * Если входное значение является NaN, фильтрация не выполняется.
     *
     * @param rawValue Сырое входное значение (например, с датчика).
     *
     * @return true  если значение успешно обработано  
     * @return false если rawValue == NaN
     */
    bool update(float rawValue);   // возвращает false если NAN

    /**
     * @brief Получение текущего отфильтрованного значения.
     *
     * @return Последнее корректное значение после фильтрации.
     */
    float value() const;

private:
    /**
     * @brief Медианный фильтр для подавления выбросов.
     */
    MedianFilter<float> median;

        /**
     * @brief Экспоненциальное скользящее среднее (EMA).
     *
     * Используется для сглаживания выходного сигнала медианного фильтра.
     */
    EMAFilter ema;

    /**
     * @brief Последнее вычисленное значение фильтра.
     */
    float lastValue;
};



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

#endif // FILTERS_H