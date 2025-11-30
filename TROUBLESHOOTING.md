# 🔧 Устранение проблем с GitHub Actions и SonarCloud

## Проблема: CMake не может найти CMakeLists.txt

### Симптомы:
```
CMake Error: The source directory "/home/runner/work/Gulevitch/Gulevitch" 
does not appear to contain CMakeLists.txt.
```

### Решения:

#### ✅ ИСПРАВЛЕНО: Проблема была в .gitignore

Файл `CMakeLists.txt` игнорировался из-за правила `*.txt` в `.gitignore`. 
**Исправление:** Добавлено исключение `!CMakeLists.txt` в `.gitignore`.

#### 1. Теперь нужно закоммитить изменения:

```bash
git add .gitignore CMakeLists.txt
git commit -m "Fix: Add CMakeLists.txt to repository (was ignored by .gitignore)"
git push
```

После этого workflow должен заработать!

#### 2. Проверьте, что CMakeLists.txt загружен в GitHub

1. Откройте ваш репозиторий на GitHub (Gulevitch)
2. Убедитесь, что файл `CMakeLists.txt` виден в корне репозитория

#### 2. Проверьте структуру репозитория

Убедитесь, что структура правильная:
```
Gulevitch/  (имя репозитория на GitHub)
├── CMakeLists.txt  ← должен быть в корне
├── src/
├── include/
├── forms/
└── .github/workflows/
```

#### 3. Проверьте .gitignore

Убедитесь, что `CMakeLists.txt` НЕ исключен в `.gitignore`:
- В `.gitignore` должна быть строка: `!CMakeLists.txt` (если есть исключение для `*.cmake`)

#### 4. Перезапустите workflow

1. GitHub → Actions
2. Найдите failed workflow
3. Нажмите "Re-run all jobs"

---

## Проблема: Workflow не запускается

### Решения:

1. Проверьте, что файл `.github/workflows/sonarcloud.yml` существует
2. Проверьте синтаксис YAML (можно проверить онлайн валидатором)
3. Убедитесь, что вы делаете push в ветку `main`, `master` или `develop`

---

## Проблема: "Authentication failed" в SonarCloud

### Решения:

1. Проверьте, что `SONAR_TOKEN_` правильно добавлен в GitHub Secrets
2. Убедитесь, что токен не истек
3. Проверьте, что имя секрета точно `SONAR_TOKEN_` (с подчеркиванием)

---

## Проблема: "Invalid project key"

### Решения:

1. Проверьте файл `sonar-project.properties`
2. Убедитесь, что `sonar.projectKey` и `sonar.organization` правильные
3. Формат: `organization-key_project-name`

---

## Проблема: "The only way to get an accurate analysis... sonar.cfamily.compile-commands"

### Симптомы:
```
ERROR Error during SonarScanner Engine execution
java.lang.UnsupportedOperationException: 
The only way to get an accurate analysis of C/C++/Objective-C files 
in Manual Configuration mode is to provide a compilation database 
through the property "sonar.cfamily.compile-commands"
```

### ✅ ИСПРАВЛЕНО: Неправильное свойство в sonar-project.properties

**Проблема:** Использовалось неправильное свойство `sonar.cfamily.compile-commands.build-dir=build`

**Решение:** Измените в `sonar-project.properties`:
```properties
# Было (неправильно):
sonar.cfamily.compile-commands.build-dir=build

# Должно быть:
sonar.cfamily.compile-commands=build/compile_commands.json
```

### Проверка:

1. Убедитесь, что `CMAKE_EXPORT_COMPILE_COMMANDS=ON` в `CMakeLists.txt`:
   ```cmake
   set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
   ```

2. Убедитесь, что CMake генерирует `compile_commands.json`:
   ```bash
   cmake -B build -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
   cmake --build build
   ls -la build/compile_commands.json  # должен существовать
   ```

3. После исправления закоммитьте изменения:
   ```bash
   git add sonar-project.properties
   git commit -m "Fix: Correct sonar.cfamily.compile-commands path"
   git push
   ```

---

## Полезные команды для отладки

### Проверка локально:

```bash
# Проверить структуру
ls -la

# Проверить наличие CMakeLists.txt
test -f CMakeLists.txt && echo "Found" || echo "Not found"

# Проверить Git статус
git status

# Проверить, что файлы не в .gitignore
git check-ignore CMakeLists.txt
```

---

## Контакты и помощь

Если проблема не решена:
1. Проверьте логи в GitHub Actions (Actions → выберите workflow → View logs)
2. Проверьте документацию SonarCloud: https://docs.sonarcloud.io/
3. Проверьте документацию GitHub Actions: https://docs.github.com/en/actions

