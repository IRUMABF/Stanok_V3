# Виправлення дублювання ініціалізації та використання датчиків

## Проблема
В коді було виявлено дублювання ініціалізації та використання датчиків між класами `Controls` та `SensorUtils`:

### Дублювання ініціалізації:
- `controls.h` (рядки 19-21): `pinMode(sensor_1, INPUT_PULLUP)`
- `sensor_utils.h` (рядки 9-11): `pinMode(sensor_1, INPUT_PULLUP)`

### Дублювання читання:
- `controls.h` (рядки 30-32): `digitalRead(sensor_1)`
- `sensor_utils.h` (рядки 20, 24, 28): `digitalRead(sensor_1)`

## Рішення

### 1. Клас `Controls` тепер відповідає за:
- Ініціалізацію всіх пінів (кнопки + датчики)
- Оновлення стану всіх кнопок та датчиків
- Надання доступу до стану кнопок та датчиків

### 2. Клас `SensorUtils` тепер містить тільки:
- Утилітарні функції для роботи з датчиками
- Функції відладки та тестування
- Функції очікування спрацювання датчиків

### 3. Зміни в `main.cpp`:
- Видалено виклик `SensorUtils::initSensors()`
- Замінено `SensorUtils::readSensor1()` на `controls.isSensor1Active()`
- Замінено `SensorUtils::readSensor2()` на `controls.isSensor2Active()`
- Замінено `SensorUtils::readSensor3()` на `controls.isSensor3Active()`
- Оновлено виклик `SensorUtils::testAllSensors()` з передачею функцій читання

## Переваги нового підходу:
1. **Єдина точка ініціалізації** - всі піни ініціалізуються в `Controls::begin()`
2. **Єдина точка оновлення** - всі датчики оновлюються в `Controls::update()`
3. **Єдиний доступ до стану** - всі методи читання датчиків в класі `Controls`
4. **Розділення відповідальності** - `Controls` для управління, `SensorUtils` для утиліт
5. **Усунуто дублювання** - кожна функція виконується тільки один раз

## Використання:
```cpp
// В setup():
controls.begin();  // Ініціалізує всі піни

// В loop():
controls.update();  // Оновлює стан всіх кнопок та датчиків

// Читання датчиків:
bool sensor1 = controls.isSensor1Active();
bool sensor2 = controls.isSensor2Active();
bool sensor3 = controls.isSensor3Active();

// Використання утиліт:
SensorUtils::debugSensors(sensor1, sensor2, sensor3);
```
