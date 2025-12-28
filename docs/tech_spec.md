# Техническое задание: Контроллер вентиляции и озонирования подвала на Arduino Nano

## 1. Общие требования

Платформа: Arduino Nano

Периферия:

BME280 (внутри)

DS18B20 (контрольный датчик подвала)

HTU21D (улица)

DS3231 RTC с аккумулятором

LCD 1602 (I2C)

3 кнопки: UP, DOWN, MENU

SSR реле: FAN, OZONE

Исполнители:

Приточный вентилятор (ON/OFF)

Озонатор (ON/OFF)

Питание: только 220В через адаптер

Вентилятор — не чаще чем раз в 5 минут (антидребезг)

Watchdog: 8 секунд

Serial debug через #define DEBUG

## 2. КЛИМАТ-КОНТРОЛЬ

Цель: T = 4°C, RH = 85%

Гистерезис: T ±0.5°C, RH ±3%

Safety margin конденсации: 2°C

Запрещено охлаждать подвал ниже +2°C

Алгоритм включения вентилятора (вариант С):

if ( (T_in > TargetTemp+hyst OR RH_in > TargetRH+hyst)
 AND (AH_out + margin < AH_in)
  AND (dewpoint_in + 2°C < T_in) ) then FAN ON
else FAN OFF


Датчики опрашивать каждые 10 секунд

Фильтры: медианный (N=3), затем EMA α=0.2

Абсолютная влажность рассчитывается только на фильтрованных данных

## 3. ОЗОНИРОВАНИЕ

Запуск по расписанию DS3231 (раз в неделю в 02:00, настраиваемый день и HH:MM)

Цикл:

OZONE ON 15 мин

Пауза 2 часа

Форсированное проветривание 15 мин

Запрещено при T_out < 0°C и при включенной подсветке дисплея

В ручном режиме подсветка игнорируется

При сбое DS3231 → ERROR, цикл не запускается

Перезапуск при запрещении → через 30 минут, далее на сутки

## 4. ОШИБКИ И ОТКАЗЫ

Критические ошибки (ручной сброс):

Несовпадение BME280 и DS18B20 > 2°C

Любая ошибка BME280

Любая ошибка HTU21D

Ошибка DS18B20

Ошибка DS3231 (3 попытки чтения)

Риск конденсации (dewpoint_in >= T_in-4°C)

T_in ≤ +2°C

Мягкие ошибки → автоматическое восстановление

## 5. ДАТЧИКИ И ОБРАБОТКА

Частота опроса: 10 сек

Фильтры: медианный (3), EMA α=0.2

Абсолютная влажность AH (г/м³) по Magnus:

svp = 6.112 * exp((17.62*T)/(243.12+T))
vp  = RH/100 * svp
AH  = 216.7 * vp / (T + 273.15)


Dewpoint для контроля конденсации и логики "outdoor air helps"

DS3231 используется для расписания озонирования и временных меток

Проверка времени при старте: при расхождении >1 мин → ERROR

## 6. FSM (State Machine)

Состояния:

IDLE

AUTO CLIMATE CONTROL

OZONE_START

OZONE_ACTIVE (15 min)

OZONE_HOLD (2h)

OZONE_VENT (15 min)

OZONE_ABORT (forced vent)

MANUAL FAN

MANUAL OZONE

ERROR

Особенности:

В фазах озонирования (active/hold) климат-логика OFF

Проветривание после озона контролируется только температурой (>0°C)

Ошибки DS3231 → ERROR, блокировка всех устройств

## 7. Меню

Status (T_in/RH_in/AH_in, T_out/RH_out/AH_out, FAN/OZONE)

Manual FAN: 15/30 min

Manual OZONE: 15/30 min

Settings: target T/RH, сенсорные offsets, расписание OZONE

Service menu: ERROR log, сброс, статистика, debug mode

Подсветка: авто/ручной override в manual OZONE

## 8. EEPROM Map (обновлённый)
Адрес	Размер	Назначение
0x00	2 байт	Target T (x10)
0x02	2 байт	Target RH (x10)
0x04	2 байт	BME280 T offset (x10)
0x06	2 байт	BME280 RH offset (x10)
0x08	2 байт	DS18B20 T offset (x10)
0x0A	2 байт	HTU21D T offset (x10)
0x0C	2 байт	HTU21D RH offset (x10)
0x0E	1 байт	OZONE schedule enable
0x0F	1 байт	OZONE weekday
0x10	2 байт	OZONE hour/min HHMM
0x12	2 байт	DS3231 calibration (ppm, signed)
0x14	32 байт	Wear leveling / config copy
0x34	64 байт	Counter snapshot FAN/OZONE hh:mm
0x74	128 байт	Event log (20 записей x 6 байт)
...	...	CRC / контрольная сумма

## 9. Skeleton кода (C++ / Arduino Nano)

Файловая структура:

/src
  main.cpp
  sensors.cpp/h
  filters.cpp/h
  climate.cpp/h
  ozone.cpp/h
  fsm.cpp/h
  eeprom.cpp/h
  menu.cpp/h
  display.cpp/h
  rtc.cpp/h      // DS3231
  utils.cpp/h
  config.h


Принципы кода:

FSM реализована через enum + switch-case

Антидребезг вентиляторного включения

Абсолютная влажность и dewpoint рассчитываются после фильтров

Watchdog 8s, loop <500ms

Ручной режим через кнопки + long-press

Serial debug через #define DEBUG
