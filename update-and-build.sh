#!/bin/bash

# Параметры
REPO_URL="https://github.com/skryabinv/splav-encoder-read-cpp.git"  
REPO_DIR="/home/user/splav-encoder-read-cpp/"                             
SERVICE_NAME="splav-encoder-read"

# Клонирование или обновление репозитория
echo "Обновляем существующий репозиторий..."
git pull || { echo "Ошибка обновления репозитория"; exit 1; }


# Переходим в каталог сборки
mkdir -p "$REPO_DIR/build"
cd "$REPO_DIR/build" || { echo "Ошибка перехода в build"; exit 1; }

# Конфигурация CMake с указанием каталога установки
cmake ..

# Сборка
make || { echo "Ошибка сборки"; exit 1; }

echo "Приложение успешно собрано в $REPO_DIR/build"

sudo systemctl restart "$SERVICE_NAME" || { echo "Не удалось запустить службу $SERVICE_NAME"; exit 1; }

# Проверим статус
if systemctl is-active --quiet "$SERVICE_NAME"; then
    echo "Служба $SERVICE_NAME успешно запущена."
else
    echo "Служба $SERVICE_NAME запущена, но статус неактивен. Проверьте: sudo systemctl status $SERVICE_NAME"
fi

# Вывод последних логов 
echo "Последние логи службы $SERVICE_NAME:"
sudo journalctl -u "$SERVICE_NAME" -n 20 --no-pager
