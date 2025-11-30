# GitHub Actions Workflows

Этот каталог содержит workflows для автоматизации CI/CD.

## Доступные workflows

### build.yml
Автоматическая сборка проекта на Ubuntu и Windows при каждом push и pull request.

### sonarcloud.yml
Автоматический анализ качества кода через SonarCloud при каждом push и pull request.

## Требования

Для работы workflows необходимо:
- Настроить SonarCloud проект (см. SETUP.md)
- Добавить `SONAR_TOKEN` в секреты GitHub репозитория

