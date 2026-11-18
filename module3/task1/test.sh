#!/bin/bash
# проверка copier при разных параметрах запуска (в том числе некорректные)

echo "Подготовка тестовых файлов"
echo "ДАРОВ! ПРАВИРЯЮ FORKZ, ФАЙЛЫ И КАНАЛЫ." > hello.txt
head -c 100000 /dev/urandom > random.bin   # 100 КБ случайных байт
: > empty.txt                              # пустой файл

echo
echo "1. Неименованный канал: обычный, бинарный, пустой и НЕсуществующий файлы"
./copier hello.txt random.bin empty.txt no_such_file.txt
cmp hello.txt hello.txt.copy && echo "OK: hello.txt совпал"
cmp random.bin random.bin.copy && echo "OK: random.bin совпал"
cmp empty.txt empty.txt.copy && echo "OK: empty.txt совпал"

echo
echo "2. Именованный канал (-p)"
./copier -p /tmp/copier_fifo hello.txt random.bin
cmp hello.txt hello.txt.copy && echo "OK: hello.txt совпал (fifo)"
cmp random.bin random.bin.copy && echo "OK: random.bin совпал (fifo)"

echo
echo "3. Некорректные параметры"
./copier              # без аргументов (должна быть подсказка об использовании)
./copier -p           # ключ без имени канала (сообщение об ошибке)
./copier /etc         # каталог вместо файла (диагностика)